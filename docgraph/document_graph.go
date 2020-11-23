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

// DocumentGraph is defined by a root node, and is aware of nodes and edges
type DocumentGraph struct {
	RootNode Document
}

// Create1P ...
type Create1P struct {
	Creator eos.AccountName `json:"creator"`
}

// Create1 ...
func Create1(ctx context.Context, api *eos.API,
	contract, creator eos.AccountName) (Document, error) {

	action := eos.ActN("create1")

	actions := []*eos.Action{
		{
			Account: contract,
			Name:    action,
			Authorization: []eos.PermissionLevel{
				{Actor: creator, Permission: eos.PN("active")},
			},
			ActionData: eos.NewActionData(Create1P{
				Creator: creator,
			}),
		}}

	_, err := eostest.ExecTrx(ctx, api, actions)
	if err != nil {
		return Document{}, fmt.Errorf("execute transaction %v", err)
	}

	lastDoc, err := GetLastDocument(ctx, api, contract)
	if err != nil {
		return Document{}, fmt.Errorf("get last document %v", err)
	}
	return lastDoc, nil
}

// CreateDocument creates a new document on chain from the provided file
func CreateDocument(ctx context.Context, api *eos.API,
	contract, creator eos.AccountName,
	fileName string) (Document, error) {

	data, err := ioutil.ReadFile(fileName)
	if err != nil {
		return Document{}, fmt.Errorf("readfile %v: %v", fileName, err)
	}

	action := eos.ActN("create")

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

// CreateEdge creates an edge from one document node to another with the specified name
func CreateEdge(ctx context.Context, api *eos.API,
	contract, creator eos.AccountName,
	fromNode, toNode eos.Checksum256,
	edgeName eos.Name) (string, error) {

	actionData := make(map[string]interface{})
	actionData["from_node"] = fromNode
	actionData["to_node"] = toNode
	actionData["edge_name"] = edgeName

	actionBinary, err := api.ABIJSONToBin(ctx, contract, eos.Name("newedge"), actionData)
	if err != nil {
		log.Println("Error with ABIJSONToBin: ", err)
		return "error", err
	}

	actions := []*eos.Action{
		{
			Account: contract,
			Name:    eos.ActN("newedge"),
			Authorization: []eos.PermissionLevel{
				{Actor: creator, Permission: eos.PN("active")},
			},
			ActionData: eos.NewActionDataFromHexData([]byte(actionBinary)),
		}}

	return eostest.ExecTrx(ctx, api, actions)
}

// EdgeExists checks to see if the edge exists
func EdgeExists(ctx context.Context, api *eos.API, contract eos.AccountName,
	fromNode, toNode Document, edgeName eos.Name) (bool, error) {

	edges, err := fromNode.GetEdgesFromByName(ctx, api, contract, edgeName)
	if err != nil {
		return false, fmt.Errorf("get edges from by name doc: %v err: %v", fromNode.Hash, err)
	}

	for _, edge := range edges {
		if edge.ToNode.String() == toNode.Hash.String() {
			return true, nil
		}
	}
	return false, nil
}

func (d *Document) getEdgesIndex(ctx context.Context, api *eos.API, contract eos.AccountName, edgeIndex string) ([]Edge, error) {
	var edges []Edge
	var request eos.GetTableRowsRequest
	request.Code = string(contract)
	request.Scope = string(contract)
	request.Table = "edges"
	request.Index = edgeIndex
	request.KeyType = "sha256"
	request.LowerBound = d.Hash.String()
	request.UpperBound = d.Hash.String()
	request.Limit = 1000
	request.JSON = true
	response, err := api.GetTableRows(ctx, request)
	if err != nil {
		log.Println("Error with GetTableRows: ", err)
		return []Edge{}, err
	}

	err = response.JSONToStructs(&edges)
	if err != nil {
		log.Println("Error with JSONToStructs: ", err)
		return []Edge{}, err
	}
	return edges, nil
}

// GetEdgesFrom retrieves a list of edges from this node to other nodes
func (d *Document) GetEdgesFrom(ctx context.Context, api *eos.API, contract eos.AccountName) ([]Edge, error) {
	return d.getEdgesIndex(ctx, api, contract, string("2"))
}

// GetEdgesTo retrieves a list of edges to this node from other nodes
func (d *Document) GetEdgesTo(ctx context.Context, api *eos.API, contract eos.AccountName) ([]Edge, error) {
	return d.getEdgesIndex(ctx, api, contract, string("3"))
}

// GetEdgesFromByName retrieves a list of edges from this node to other nodes
func (d *Document) GetEdgesFromByName(ctx context.Context, api *eos.API, contract eos.AccountName, edgeName eos.Name) ([]Edge, error) {
	edges, err := d.getEdgesIndex(ctx, api, contract, string("2"))
	if err != nil {
		log.Println("Error with JSONToStructs: ", err)
		return []Edge{}, err
	}

	var namedEdges []Edge
	for _, edge := range edges {
		if edge.EdgeName == edgeName {
			namedEdges = append(namedEdges, edge)
		}
	}
	return namedEdges, nil
}

// GetEdgesToByName retrieves a list of edges from this node to other nodes
func (d *Document) GetEdgesToByName(ctx context.Context, api *eos.API, contract eos.AccountName, edgeName eos.Name) ([]Edge, error) {
	edges, err := d.getEdgesIndex(ctx, api, contract, string("3"))
	if err != nil {
		log.Println("Error with JSONToStructs: ", err)
		return []Edge{}, err
	}

	var namedEdges []Edge
	for _, edge := range edges {
		if edge.EdgeName == edgeName {
			namedEdges = append(namedEdges, edge)
		}
	}
	return namedEdges, nil
}

// GetLastDocument retrieves the last document that was created from the contract
func GetLastDocument(ctx context.Context, api *eos.API, contract eos.AccountName) (Document, error) {
	var docs []Document
	var request eos.GetTableRowsRequest
	request.Code = string(contract)
	request.Scope = string(contract)
	request.Table = "documents"
	request.Reverse = true
	request.Limit = 1
	request.JSON = true
	response, err := api.GetTableRows(ctx, request)
	if err != nil {
		log.Println("Error with GetTableRows: ", err)
		return Document{}, err
	}

	err = response.JSONToStructs(&docs)
	if err != nil {
		log.Println("Error with JSONToStructs: ", err)
		return Document{}, err
	}
	return docs[0], nil
}
