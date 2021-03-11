package docgraph

import (
	"context"
	"fmt"
	"log"
	"strconv"

	eos "github.com/eoscanada/eos-go"
	"github.com/k0kubun/go-ansi"
	"github.com/schollz/progressbar/v3"
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
	request.Limit = 10000
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

// GetDocumentsWithEdge retrieves a list of documents connected to the provided document via the provided edge name
func GetDocumentsWithEdge(ctx context.Context, api *eos.API, contract eos.AccountName, document Document, edgeName eos.Name) ([]Document, error) {
	edges, err := GetEdgesFromDocumentWithEdge(ctx, api, contract, document, edgeName)
	if err != nil {
		return []Document{}, fmt.Errorf("error retrieving edges %v", err)
	}

	documents := make([]Document, len(edges))
	for index, edge := range edges {
		documents[index], err = LoadDocument(ctx, api, contract, edge.ToNode.String())
		if err != nil {
			return []Document{}, fmt.Errorf("error loading document %v", err)
		}
	}
	return documents, nil
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
	// request.LowerBound = sdtrinedgeName
	// request.UpperBound = edgeName
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

	return Document{}, fmt.Errorf("no document with edge found: %v", string(edgeName))
}

func getRange(ctx context.Context, api *eos.API, contract eos.AccountName, id, count int) ([]Document, bool, error) {
	var documents []Document
	var request eos.GetTableRowsRequest
	if id > 0 {
		request.LowerBound = strconv.Itoa(id)
	}
	request.Code = string(contract)
	request.Scope = string(contract)
	request.Table = "documents"
	request.Limit = uint32(count)
	request.JSON = true
	response, err := api.GetTableRows(ctx, request)
	if err != nil {
		return []Document{}, false, fmt.Errorf("get table rows %v", err)
	}

	err = response.JSONToStructs(&documents)
	if err != nil {
		return []Document{}, false, fmt.Errorf("json to structs %v", err)
	}
	return documents, response.More, nil
}

// GetAllDocumentsForType reads all documents and returns them in a slice
func GetAllDocumentsForType(ctx context.Context, api *eos.API, contract eos.AccountName, docType string) ([]Document, error) {

	allDocuments, err := GetAllDocuments(ctx, api, contract)
	if err != nil {
		return []Document{}, fmt.Errorf("cannot get all documents %v", err)
	}

	var filteredDocs []Document
	for _, doc := range allDocuments {

		typeFV, err := doc.GetContent("type")
		if err == nil &&
			typeFV.Impl.(eos.Name) == eos.Name(docType) {
			filteredDocs = append(filteredDocs, doc)
		}
	}

	return filteredDocs, nil
}

// GetAllDocuments reads all documents and returns them in a slice
func GetAllDocuments(ctx context.Context, api *eos.API, contract eos.AccountName) ([]Document, error) {

	var allDocuments []Document
	batchSize := 75

	bar := DefaultProgressBar("Retrieving graph for cache ... ", -1) // progressbar.Default(-1)

	batch, more, err := getRange(ctx, api, contract, 0, batchSize)
	if err != nil {
		return []Document{}, fmt.Errorf("json to structs %v", err)
	}
	allDocuments = append(allDocuments, batch...)
	bar.Add(batchSize)

	for more {
		batch, more, err = getRange(ctx, api, contract, int(batch[len(batch)-1].ID), batchSize)
		if err != nil {
			return []Document{}, fmt.Errorf("json to structs %v", err)
		}
		allDocuments = append(allDocuments, batch...)
		bar.Add(batchSize)
	}

	bar.Clear()
	return allDocuments, nil
}

func DefaultProgressBar(prefix string, counter int) *progressbar.ProgressBar {
	return progressbar.NewOptions(counter,
		progressbar.OptionSetWriter(ansi.NewAnsiStdout()),
		progressbar.OptionEnableColorCodes(true),
		progressbar.OptionSetWidth(90),
		// progressbar.OptionShowIts(),
		progressbar.OptionSetDescription("[cyan]"+fmt.Sprintf("%20v", prefix)),
		progressbar.OptionSetTheme(progressbar.Theme{
			Saucer:        "[green]=[reset]",
			SaucerHead:    "[green]>[reset]",
			SaucerPadding: " ",
			BarStart:      "[",
			BarEnd:        "]",
		}))
}
