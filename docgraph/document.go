package docgraph

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"strings"

	eostest "github.com/digital-scarcity/eos-go-test"
	eos "github.com/eoscanada/eos-go"
)

// FlexValueVariant may hold a name, int64, asset, string, or time_point
var FlexValueVariant = eos.NewVariantDefinition([]eos.VariantType{
	{Name: "monostate", Type: int64(0)},
	{Name: "name", Type: eos.Name("")},
	{Name: "string", Type: ""},
	{Name: "asset", Type: (*eos.Asset)(nil)}, // Syntax for pointer to a type, could be any struct
	{Name: "time_point", Type: eos.TimePoint(0)},
	{Name: "int64", Type: int64(0)},
	{Name: "checksum256", Type: eos.Checksum256([]byte("0"))},
})

// GetVariants returns the definition of types compatible with FlexValue
func GetVariants() *eos.VariantDefinition {
	return FlexValueVariant
}

// FlexValue may hold any of the common EOSIO types
// name, int64, asset, string, time_point, or checksum256
type FlexValue struct {
	eos.BaseVariant
}

func (fv *FlexValue) String() string {
	switch v := fv.Impl.(type) {
	case eos.Name:
		return string(v)
	case int64:
		return fmt.Sprint(v)
	case *eos.Asset:
		return v.String()
	case string:
		return v
	case eos.TimePoint:
		return v.String()
	case eos.Checksum256:
		return v.String()
	default:
		panic(fmt.Errorf("received an unexpected type %T for metadata variant %T", v, fv))
	}
}

// IsEqual evaluates if the two FlexValues have the same types and values (deep compare)
func (fv *FlexValue) IsEqual(fv2 *FlexValue) bool {

	if fv.TypeID != fv2.TypeID {
		log.Println("FlexValue types inequal: ", fv.TypeID, " vs ", fv2.TypeID)
		return false
	}

	if fv.String() != fv2.String() {
		log.Println("FlexValue Values.String() inequal: ", fv.String(), " vs ", fv2.String())
		return false
	}

	return true
}

// MarshalJSON translates to []byte
func (fv *FlexValue) MarshalJSON() ([]byte, error) {
	return fv.BaseVariant.MarshalJSON(FlexValueVariant)
}

// UnmarshalJSON translates flexValueVariant
func (fv *FlexValue) UnmarshalJSON(data []byte) error {
	return fv.BaseVariant.UnmarshalJSON(data, FlexValueVariant)
}

// UnmarshalBinary ...
func (fv *FlexValue) UnmarshalBinary(decoder *eos.Decoder) error {
	return fv.BaseVariant.UnmarshalBinaryVariant(decoder, FlexValueVariant)
}

// ContentNotFoundError is used when content matching
// a specific label is requested but not found in a document
type ContentNotFoundError struct {
	Label        string
	DocumentHash eos.Checksum256
}

func (c *ContentNotFoundError) Error() string {
	return fmt.Sprintf("content label not found: %v in document: %v", c.Label, c.DocumentHash.String())
}

// ContentItem ...
type ContentItem struct {
	Label string     `json:"label"`
	Value *FlexValue `json:"value"`
}

// IsEqual evalutes if the label, value type and value impl are the same
func (c *ContentItem) IsEqual(c2 ContentItem) bool {
	if strings.Compare(c.Label, c2.Label) != 0 {
		log.Println("ContentItem labels inequal: ", c.Label, " vs ", c2.Label)
		return false
	}

	if !c.Value.IsEqual(c2.Value) {
		log.Println("ContentItems inequal: ", c.Value, " vs ", c2.Value)
		return false
	}

	return true
}

// ContentGroup ...
type ContentGroup []ContentItem

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

	_, err = eostest.ExecTrx(ctx, api, actions)
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

// IsEqual is a deep equal comparison of two documents
//
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
	return eostest.ExecTrx(ctx, api, actions)
}
