package docgraph

import (
	"context"
	"fmt"
	"log"

	eos "github.com/eoscanada/eos-go"
)

// DocumentGraph is defined by a root node, and is aware of nodes and edges
type DocumentGraph struct {
	RootNode Document
}

func getEdgesByIndex(ctx context.Context, api *eos.API, contract eos.AccountName, document Document, edgeIndex string) ([]Edge, error) {
	var edges []Edge
	var request eos.GetTableRowsRequest
	request.Code = string(contract)
	request.Scope = string(contract)
	request.Table = "edges"
	request.Index = edgeIndex
	request.KeyType = "sha256"
	request.LowerBound = document.Hash.String()
	request.UpperBound = document.Hash.String()
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

// GetEdgesFromDocument retrieves a list of edges from this node to other nodes
func GetEdgesFromDocument(ctx context.Context, api *eos.API, contract eos.AccountName, document Document) ([]Edge, error) {
	return getEdgesByIndex(ctx, api, contract, document, string("2"))
}

// GetEdgesToDocument retrieves a list of edges to this node from other nodes
func GetEdgesToDocument(ctx context.Context, api *eos.API, contract eos.AccountName, document Document) ([]Edge, error) {
	return getEdgesByIndex(ctx, api, contract, document, string("3"))
}

// GetEdgesFromDocumentWithEdge retrieves a list of edges from this node to other nodes
func GetEdgesFromDocumentWithEdge(ctx context.Context, api *eos.API, contract eos.AccountName, document Document, edgeName eos.Name) ([]Edge, error) {
	edges, err := getEdgesByIndex(ctx, api, contract, document, string("2"))
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

// GetEdgesToDocumentWithEdge retrieves a list of edges from this node to other nodes
func GetEdgesToDocumentWithEdge(ctx context.Context, api *eos.API, contract eos.AccountName, document Document, edgeName eos.Name) ([]Edge, error) {
	edges, err := getEdgesByIndex(ctx, api, contract, document, string("3"))
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

// GetLastDocumentOfEdge ...
func GetLastDocumentOfEdge(ctx context.Context, api *eos.API, contract eos.AccountName, edgeName eos.Name) (Document, error) {
	var edges []Edge
	var request eos.GetTableRowsRequest
	request.Code = string(contract)
	request.Scope = string(contract)
	request.Table = "edges"
	request.Reverse = true
	request.Index = "8"
	request.KeyType = "i64"
	request.Limit = 1000
	request.JSON = true
	response, err := api.GetTableRows(ctx, request)
	if err != nil {
		return Document{}, fmt.Errorf("json to struct: %v", err)
	}

	err = response.JSONToStructs(&edges)
	if err != nil {
		return Document{}, fmt.Errorf("json to struct: %v", err)
	}

	for _, edge := range edges {
		if edge.EdgeName == edgeName {
			return LoadDocument(ctx, api, contract, edge.ToNode.String())
		}
	}

	return Document{}, fmt.Errorf("no proposal found")
}
