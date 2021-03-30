package docgraph

import (
	"context"
	"fmt"
	"log"
	"strconv"

	eostest "github.com/digital-scarcity/eos-go-test"
	eos "github.com/eoscanada/eos-go"
)

// Edge is a directional, named connection from one graph to another
type Edge struct {
	ID          uint64             `json:"id"`
	Creator     eos.Name           `json:"creator"`
	FromNode    eos.Checksum256    `json:"from_node"`
	ToNode      eos.Checksum256    `json:"to_node"`
	EdgeName    eos.Name           `json:"edge_name"`
	CreatedDate eos.BlockTimestamp `json:"created_date"`
}

// RemoveEdges ...
type RemoveEdges struct {
	FromNode eos.Checksum256 `json:"from_node"`
	EdgeName eos.Name        `json:"edge_name"`
	Strict   bool            `json:"strict"`
}

type removeEdge struct {
	FromNode eos.Checksum256 `json:"from_node"`
	ToNode   eos.Checksum256 `json:"to_node"`
	EdgeName eos.Name        `json:"edge_name"`
}

// CreateEdge creates an edge from one document node to another with the specified name
func CreateEdge(ctx context.Context, api *eos.API,
	contract, creator eos.AccountName,
	fromNode, toNode eos.Checksum256,
	edgeName eos.Name) (string, error) {

	actionData := make(map[string]interface{})
	actionData["creator"] = creator
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

	return eostest.ExecWithRetry(ctx, api, actions)
}

// RemoveEdge ...
func RemoveEdge(ctx context.Context, api *eos.API,
	contract eos.AccountName,
	fromHash, toHash eos.Checksum256, edgeName eos.Name) (string, error) {

	actions := []*eos.Action{{
		Account: contract,
		Name:    eos.ActN("removeedge"),
		Authorization: []eos.PermissionLevel{
			{Actor: contract, Permission: eos.PN("active")},
		},
		ActionData: eos.NewActionData(removeEdge{
			FromNode: fromHash,
			ToNode:   toHash,
			EdgeName: edgeName,
		}),
	}}
	return eostest.ExecWithRetry(ctx, api, actions)
}

// // GetAllEdges retrieves all edges from table
// func GetAllEdges(ctx context.Context, api *eos.API, contract eos.AccountName) ([]Edge, error) {
// 	var edges []Edge
// 	var request eos.GetTableRowsRequest
// 	request.Code = string(contract)
// 	request.Scope = string(contract)
// 	request.Table = "edges"
// 	request.Limit = 100000
// 	request.JSON = true
// 	response, err := api.GetTableRows(ctx, request)
// 	if err != nil {
// 		log.Println("Error with GetTableRows: ", err)
// 		return []Edge{}, err
// 	}

// 	err = response.JSONToStructs(&edges)
// 	if err != nil {
// 		log.Println("Error with JSONToStructs: ", err)
// 		return []Edge{}, err
// 	}
// 	return edges, nil
// }

func getEdgeRange(ctx context.Context, api *eos.API, contract eos.AccountName, id, count int) ([]Edge, bool, error) {
	var edges []Edge
	var request eos.GetTableRowsRequest

	if id > 0 {
		request.LowerBound = strconv.Itoa(id)
	}
	request.Code = string(contract)
	request.Scope = string(contract)
	request.Table = "edges"
	request.Limit = uint32(count)
	request.JSON = true
	response, err := api.GetTableRows(ctx, request)
	if err != nil {
		return []Edge{}, false, fmt.Errorf("retrieving edge range %v", err)
	}

	err = response.JSONToStructs(&edges)
	if err != nil {
		return []Edge{}, false, fmt.Errorf("edge json to structs %v", err)
	}
	return edges, response.More, nil
}

// GetAllEdges reads all documents and returns them in a slice
func GetAllEdges(ctx context.Context, api *eos.API, contract eos.AccountName) ([]Edge, error) {

	var allEdges []Edge

	batchSize := 1000

	batch, more, err := getEdgeRange(ctx, api, contract, 0, batchSize)
	if err != nil {
		return []Edge{}, fmt.Errorf("cannot get initial range of edges %v", err)
	}
	allEdges = append(allEdges, batch...)

	for more {
		batch, more, err = getEdgeRange(ctx, api, contract, int(batch[len(batch)-1].ID), batchSize)
		if err != nil {
			return []Edge{}, fmt.Errorf("cannot get range of edges %v", err)
		}
		allEdges = append(allEdges, batch...)
	}

	return allEdges, nil
}

// EdgeExists checks to see if the edge exists
func EdgeExists(ctx context.Context, api *eos.API, contract eos.AccountName,
	fromNode, toNode Document, edgeName eos.Name) (bool, error) {

	edges, err := GetAllEdges(ctx, api, contract)
	if err != nil {
		return false, fmt.Errorf("get edges from by name doc: %v err: %v", fromNode.Hash, err)
	}

	for _, edge := range edges {
		if edge.ToNode.String() == toNode.Hash.String() &&
			edge.FromNode.String() == fromNode.Hash.String() &&
			edge.EdgeName == edgeName {
			return true, nil
		}
	}
	return false, nil
}

// RemoveEdgesFromAndName ...
// func RemoveEdgesFromAndName(ctx context.Context, api *eos.API,
// 	contract eos.AccountName,
// 	fromHash eos.Checksum256, edgeName eos.Name) (string, error) {

// 	actions := []*eos.Action{{
// 		Account: contract,
// 		Name:    eos.ActN("remedgesfn"),
// 		Authorization: []eos.PermissionLevel{
// 			{Actor: contract, Permission: eos.PN("active")},
// 		},
// 		ActionData: eos.NewActionData(RemoveEdges{
// 			FromNode: fromHash,
// 			EdgeName: edgeName,
// 			Strict:   true,
// 		}),
// 	}}
// 	return eostest.ExecWithRetry(ctx, api, actions)
// }

// RemoveEdgesFromAndTo ...
// func RemoveEdgesFromAndTo(ctx context.Context, api *eos.API,
// 	contract eos.AccountName,
// 	fromHash, toHash eos.Checksum256) (string, error) {

// 	actions := []*eos.Action{{
// 		Account: contract,
// 		Name:    eos.ActN("remedgesft"),
// 		Authorization: []eos.PermissionLevel{
// 			{Actor: contract, Permission: eos.PN("active")},
// 		},
// 		ActionData: eos.NewActionData(removeEdgesFT{
// 			FromNode: fromHash,
// 			ToNode:   toHash,
// 			Strict:   true,
// 		}),
// 	}}
// 	return eostest.ExecWithRetry(ctx, api, actions)
// }
