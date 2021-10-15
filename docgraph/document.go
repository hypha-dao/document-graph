package docgraph

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"

	eostest "github.com/digital-scarcity/eos-go-test"
	eos "github.com/eoscanada/eos-go"
)

// ContentNotFoundError is used when content matching
// a specific label is requested but not found in a document
type ContentNotFoundError struct {
	Label        string
	DocumentHash eos.Checksum256
}

func (c *ContentNotFoundError) Error() string {
	return fmt.Sprintf("content label not found: %v in document: %v", c.Label, c.DocumentHash.String())
}

// Document is a node in the document graph
// A document may hold any arbitrary, EOSIO compatible data
type Document struct {
	ID            uint64          `json:"id"`
	Hash          eos.Checksum256 `json:"hash"`
	Creator       eos.AccountName `json:"creator"`
	ContentGroups []ContentGroup  `json:"content_groups"`
	Certificates  []struct {
		Certifier         eos.AccountName    `json:"certifier"`
		Notes             string             `json:"notes"`
		CertificationDate eos.BlockTimestamp `json:"certification_date"`
	} `json:"certificates"`
	CreatedDate eos.BlockTimestamp `json:"created_date"`
}

func newDocumentTrx(ctx context.Context, api *eos.API,
	contract, creator eos.AccountName, actionName,
	fileName string) (Document, error) {

	data, err := ioutil.ReadFile(fileName)
	if err != nil {
		return Document{}, fmt.Errorf("readfile %v: %v", fileName, err)
	}

	action := eos.ActN(actionName)

	var dump map[string]interface{}
	err = json.Unmarshal(data, &dump)
	if err != nil {
		return Document{}, fmt.Errorf("unmarshal %v: %v", fileName, err)
	}

	dump["creator"] = creator

	actionBinary, err := api.ABIJSONToBin(ctx, contract, eos.Name(action), dump)
	if err != nil {
		return Document{}, fmt.Errorf("api json to bin %v: %v", fileName, err)
	}

	actions := []*eos.Action{
		{
			Account: contract,
			Name:    action,
			Authorization: []eos.PermissionLevel{
				{Actor: creator, Permission: eos.PN("active")},
			},
			ActionData: eos.NewActionDataFromHexData([]byte(actionBinary)),
		}}

	_, err = eostest.ExecWithRetry(ctx, api, actions)
	if err != nil {
		return Document{}, fmt.Errorf("execute transaction %v: %v", fileName, err)
	}

	lastDoc, err := GetLastDocument(ctx, api, contract)
	if err != nil {
		return Document{}, fmt.Errorf("get last document %v: %v", fileName, err)
	}
	return lastDoc, nil
}

// CreateDocument creates a new document on chain from the provided file
func CreateDocument(ctx context.Context, api *eos.API,
	contract, creator eos.AccountName,
	fileName string) (Document, error) {

	return newDocumentTrx(ctx, api, contract, creator, "create", fileName)
}

// GetContent returns a FlexValue of the content with the matching label
// or an instance of ContentNotFoundError
func (d *Document) GetContent(label string) (*FlexValue, error) {
	for _, contentGroup := range d.ContentGroups {
		for _, content := range contentGroup {
			if content.Label == label {
				return content.Value, nil
			}
		}
	}
	return nil, &ContentNotFoundError{
		Label:        label,
		DocumentHash: d.Hash,
	}
}

// GetContentFromGroup returns a FlexValue of the content with the matching label
// or an instance of ContentNotFoundError
func (d *Document) GetContentFromGroup(groupLabel, label string) (*FlexValue, error) {
	for _, contentGroup := range d.ContentGroups {
		for _, content := range contentGroup {
			if content.Label == label {
				return content.Value, nil
			}
		}
	}
	return nil, &ContentNotFoundError{
		Label:        label,
		DocumentHash: d.Hash,
	}
}

// GetContentGroup returns a ContentGroup matching the label
// or an instance of ContentNotFoundError
func (d *Document) GetContentGroup(label string) (*ContentGroup, error) {
	for _, contentGroup := range d.ContentGroups {
		for _, content := range contentGroup {
			if content.Label == "content_group_label" {
				if content.Value.String() == label {
					return &contentGroup, nil
				}
				break // found label but wrong value, go to next group
			}
		}
	}
	return nil, &ContentNotFoundError{
		Label:        label,
		DocumentHash: d.Hash,
	}
}

// GetNodeLabel returns a string for the node label
func (d *Document) GetNodeLabel() string {
	nodeLabel, err := d.GetContentFromGroup("system", "node_label")
	if err != nil {
		return ""
	}
	return nodeLabel.String()
}

// GetType return the document type; fails if it does not exist or is not an eos.Name type
func (d *Document) GetType() (eos.Name, error) {
	typeValue, err := d.GetContentFromGroup("system", "type")
	if err != nil {
		return eos.Name(""), nil
		// return eos.Name(""), fmt.Errorf("document type does not exist in system group of document: %v", err)
	}

	typeValueName, err := typeValue.Name()
	if err != nil {
		return eos.Name(""), fmt.Errorf("document type is not an eos.Name value: %v", err)
	}

	return typeValueName, nil
}

// IsEqual is a deep equal comparison of two documents
func (d *Document) IsEqual(d2 Document) bool {

	// ensure the same number of content groups
	if len(d.ContentGroups) != len(d2.ContentGroups) {
		log.Println("ContentGroups lengths inequal: ", len(d.ContentGroups), " vs ", len(d2.ContentGroups))
		return false
	}

	for contentGroupIndex, contentGroup1 := range d.ContentGroups {
		contentGroup2 := d2.ContentGroups[contentGroupIndex]

		// ensure these two content groups have the same number of items
		if len(contentGroup1) != len(contentGroup2) {
			log.Println("ContentGroup lengths inequal for CG index: ", contentGroupIndex, "; ", len(contentGroup1), " vs ", len(contentGroup2))
			return false
		}

		for contentIndex, content1 := range contentGroup1 {
			content2 := contentGroup2[contentIndex]

			// ensure these content items have the same label and same value
			if !content1.IsEqual(content2) {
				return false
			}
		}
	}

	// if we got through all the above checks, the documents are equal
	return true
}

// LoadDocument reads a document from the blockchain and creates a Document instance
func LoadDocument(ctx context.Context, api *eos.API,
	contract eos.AccountName,
	hash string) (Document, error) {

	var documents []Document
	var request eos.GetTableRowsRequest
	request.Code = string(contract)
	request.Scope = string(contract)
	request.Table = "documents"
	request.Index = "2"
	request.KeyType = "sha256"
	request.LowerBound = hash
	request.UpperBound = hash
	request.Limit = 1
	request.JSON = true
	response, err := api.GetTableRows(ctx, request)
	if err != nil {
		return Document{}, fmt.Errorf("get table rows %v: %v", hash, err)
	}

	err = response.JSONToStructs(&documents)
	if err != nil {
		return Document{}, fmt.Errorf("json to structs %v: %v", hash, err)
	}

	if len(documents) == 0 {
		return Document{}, fmt.Errorf("document not found %v: %v", hash, err)
	}
	return documents[0], nil
}

type eraseDoc struct {
	Hash eos.Checksum256 `json:"hash"`
}

// EraseDocument ...
func EraseDocument(ctx context.Context, api *eos.API,
	contract eos.AccountName,
	hash eos.Checksum256) (string, error) {

	actions := []*eos.Action{{
		Account: contract,
		Name:    eos.ActN("erase"),
		Authorization: []eos.PermissionLevel{
			{Actor: contract, Permission: eos.PN("active")},
		},
		ActionData: eos.NewActionData(eraseDoc{
			Hash: hash,
		}),
	}}
	return eostest.ExecWithRetry(ctx, api, actions)
}
