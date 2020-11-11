package docgraph_test

import (
	"context"
	"fmt"
	"io/ioutil"
	"log"
	"math/rand"
	"testing"
	"time"

	eostest "github.com/digital-scarcity/eos-go-test"
	eos "github.com/eoscanada/eos-go"
	"github.com/hypha-dao/document/docgraph"
	"github.com/k0kubun/go-ansi"
	"github.com/schollz/progressbar/v3"
	"gotest.tools/assert"
)

type createDoc struct {
	Creator       eos.AccountName         `json:"creator"`
	ContentGroups []docgraph.ContentGroup `json:"content_groups"`
}

var seededRand *rand.Rand = rand.New(
	rand.NewSource(time.Now().UnixNano()))

func stringWithCharset(length int, charset string) string {
	b := make([]byte, length)
	for i := range b {
		b[i] = charset[seededRand.Intn(len(charset))]
	}
	return string(b)
}

const charset = "abcdefghijklmnopqrstuvwxyz" + "12345"

func randomString() string {
	return stringWithCharset(12, charset)
}

// CreateRandomDocument creates a document with a single random value
func CreateRandomDocument(ctx context.Context, api *eos.API, contract, creator eos.AccountName) (docgraph.Document, error) {

	var ci docgraph.ContentItem
	ci.Label = randomString()
	ci.Value = &docgraph.FlexValue{
		BaseVariant: eos.BaseVariant{
			TypeID: docgraph.FlexValueVariant.TypeID("name"),
			Impl:   randomString(),
		},
	}

	cg := make([]docgraph.ContentItem, 1)
	cg[0] = ci
	cgs := make([]docgraph.ContentGroup, 1)
	cgs[0] = cg

	actions := []*eos.Action{{
		Account: contract,
		Name:    eos.ActN("create"),
		Authorization: []eos.PermissionLevel{
			{Actor: creator, Permission: eos.PN("active")},
		},
		ActionData: eos.NewActionData(createDoc{
			Creator:       creator,
			ContentGroups: cgs,
		}),
	}}
	_, err := eostest.ExecTrx(ctx, api, actions)
	if err != nil {
		return docgraph.Document{}, fmt.Errorf("execute transaction random document: %v", err)
	}

	lastDoc, err := docgraph.GetLastDocument(ctx, api, contract)
	if err != nil {
		return docgraph.Document{}, fmt.Errorf("get last document: %v", err)
	}
	return lastDoc, nil
}

// GetAllEdges retrieves all edges from table
func GetAllEdges(ctx context.Context, api *eos.API, contract eos.AccountName) ([]docgraph.Edge, error) {
	var edges []docgraph.Edge
	var request eos.GetTableRowsRequest
	request.Code = string(contract)
	request.Scope = string(contract)
	request.Table = "edges"
	request.Limit = 1000
	request.JSON = true
	response, err := api.GetTableRows(ctx, request)
	if err != nil {
		log.Println("Error with GetTableRows: ", err)
		return []docgraph.Edge{}, err
	}

	err = response.JSONToStructs(&edges)
	if err != nil {
		log.Println("Error with JSONToStructs: ", err)
		return []docgraph.Edge{}, err
	}
	return edges, nil
}

func pause(t *testing.T, seconds time.Duration, headline, prefix string) {
	if headline != "" {
		t.Log(headline)
	}

	bar := progressbar.NewOptions(100,
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

	chunk := seconds / 100
	for i := 0; i < 100; i++ {
		bar.Add(1)
		time.Sleep(chunk)
	}
	fmt.Println()
	fmt.Println()
}

func SaveGraph(ctx context.Context, api *eos.API, contract eos.AccountName, folderName string) error {

	var request eos.GetTableRowsRequest
	request.Code = string(contract)
	request.Scope = string(contract)
	request.Table = "documents"
	request.Limit = 1000
	request.JSON = true
	response, err := api.GetTableRows(ctx, request)
	if err != nil {
		return fmt.Errorf("Unable to retrieve rows: %v", err)
	}

	data, err := response.Rows.MarshalJSON()
	if err != nil {
		return fmt.Errorf("Unable to marshal json: %v", err)
	}

	documentsFile := folderName + "/documents.json"
	err = ioutil.WriteFile(documentsFile, data, 0644)
	if err != nil {
		return fmt.Errorf("Unable to write file: %v", err)
	}

	request = eos.GetTableRowsRequest{}
	request.Code = string(contract)
	request.Scope = string(contract)
	request.Table = "edges"
	request.Limit = 1000
	request.JSON = true
	response, err = api.GetTableRows(ctx, request)
	if err != nil {
		return fmt.Errorf("Unable to retrieve rows: %v", err)
	}

	data, err = response.Rows.MarshalJSON()
	if err != nil {
		return fmt.Errorf("Unable to marshal json: %v", err)
	}

	edgesFile := folderName + "/edges.json"
	err = ioutil.WriteFile(edgesFile, data, 0644)
	if err != nil {
		return fmt.Errorf("Unable to write file: %v", err)
	}

	return nil
}

func checkEdge(t *testing.T, env *Environment, fromEdge, toEdge docgraph.Document, edgeName eos.Name) {
	exists, err := docgraph.EdgeExists(env.ctx, &env.api, env.Docs, fromEdge, toEdge, edgeName)
	assert.NilError(t, err)
	if !exists {
		t.Log("Edge does not exist	: ", fromEdge.Hash.String(), "	-- ", edgeName, "	--> 	", toEdge.Hash.String())
	}
	assert.Check(t, exists)
}

// this function/action will remove all edges with the from node and edge name
func EdgeIdxTest(ctx context.Context, api *eos.API,
	contract eos.AccountName,
	fromHash eos.Checksum256, edgeName eos.Name) (string, error) {

	actions := []*eos.Action{{
		Account: contract,
		Name:    eos.ActN("testedgeidx"),
		Authorization: []eos.PermissionLevel{
			{Actor: contract, Permission: eos.PN("active")},
		},
		ActionData: eos.NewActionData(docgraph.RemoveEdges{
			FromNode: fromHash,
			EdgeName: edgeName,
			Strict:   true,
		}),
	}}
	return eostest.ExecTrx(ctx, api, actions)
}
