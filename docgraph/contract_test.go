package docgraph

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"testing"

	eostest "github.com/digital-scarcity/eos-go-test"
	eos "github.com/eoscanada/eos-go"
	"github.com/stretchr/testify/suite"
)

const testingEndpoint = "http://localhost:8888"

func getLastDocument(ctx context.Context, api *eos.API, contract eos.AccountName) (Document, error) {
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

func getEdges(ctx context.Context, api *eos.API, contract eos.AccountName) ([]Edge, error) {
	var edges []Edge
	var request eos.GetTableRowsRequest
	request.Code = string(contract)
	request.Scope = string(contract)
	request.Table = "edges"
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

type ContractTestSuite struct {
	suite.Suite
	Accounts     []eos.AccountName
	DocsContract eos.AccountName
	api          *eos.API
	ctx          context.Context
}

func (suite *ContractTestSuite) SetupTest() {
	fmt.Println()
	fmt.Println("**************  Setting up the test environment *****************")
	fmt.Println()

	suite.api = eos.New(testingEndpoint)
	//api.Debug = true
	suite.ctx = context.Background()

	keyBag := &eos.KeyBag{}
	err := keyBag.ImportPrivateKey(suite.ctx, eostest.DefaultKey())
	if err != nil {
		log.Panicf("cannot import default private key: %s", err)
	}
	suite.api.SetSigner(keyBag)

	suite.Accounts, err = eostest.CreateRandoms(suite.ctx, suite.api, 20)
	if err != nil {
		log.Panicf("cannot create random accounts: %s", err)
	}

	suite.DocsContract = suite.Accounts[0]

	trxID, err := eostest.SetContract(suite.ctx, suite.api, &suite.DocsContract, "../docs/docs.wasm", "../docs/docs.abi")
	if err != nil {
		log.Panicf("cannot set contract: %s", err)
	}
	log.Println("Set Docs contract	: ", suite.DocsContract, " : ", trxID)
}

type createDoc struct {
	Creator       eos.AccountName `json:"creator"`
	ContentGroups []ContentGroup  `json:"content_groups"`
}

func (suite *ContractTestSuite) createDocument(fileName string) Document {
	data, err := ioutil.ReadFile(fileName)
	suite.Require().NoError(err)

	action := eostest.ToActionName("create", "action")

	var dump map[string]interface{}
	err = json.Unmarshal(data, &dump)
	suite.Require().NoError(err)

	dump["creator"] = suite.DocsContract

	actionBinary, err := suite.api.ABIJSONToBin(suite.ctx, suite.DocsContract, eos.Name(action), dump)

	actions := []*eos.Action{
		{
			Account: suite.DocsContract,
			Name:    action,
			Authorization: []eos.PermissionLevel{
				{Actor: suite.DocsContract, Permission: eos.PN("active")},
			},
			ActionData: eos.NewActionDataFromHexData([]byte(actionBinary)),
		}}

	_, err = eostest.ExecTrx(suite.ctx, suite.api, actions)
	suite.Require().NoError(err)

	lastDoc, err := getLastDocument(suite.ctx, suite.api, suite.DocsContract)
	suite.Assert().Equal(lastDoc.Creator, eos.Name(suite.DocsContract))
	return lastDoc
}

func (suite *ContractTestSuite) createEdge(fromNode, toNode eos.Checksum256, edgeName eos.Name) {

	action := eostest.ToActionName("newedge", "action")

	actionData := make(map[string]interface{})
	actionData["from_node"] = fromNode
	actionData["to_node"] = toNode
	actionData["edge_name"] = edgeName

	actionBinary, err := suite.api.ABIJSONToBin(suite.ctx, suite.DocsContract, eos.Name(action), actionData)

	actions := []*eos.Action{
		{
			Account: suite.DocsContract,
			Name:    action,
			Authorization: []eos.PermissionLevel{
				{Actor: suite.DocsContract, Permission: eos.PN("active")},
			},
			ActionData: eos.NewActionDataFromHexData([]byte(actionBinary)),
		}}

	_, err = eostest.ExecTrx(suite.ctx, suite.api, actions)
	suite.Require().NoError(err)
}

func (suite *ContractTestSuite) TestDocuments() {

	tests := []struct {
		name  string
		input string
	}{
		{
			name:  "simplest",
			input: "../test/examples/simplest.json",
		},
		{
			name:  "each-type",
			input: "../test/examples/each-type.json",
		},
		{
			name:  "contribution",
			input: "../test/examples/contribution.json",
		},
	}

	for _, test := range tests {
		suite.Run(test.name, func() {

			lastDoc := suite.createDocument(test.input)

			// unmarshal JSON into a Document
			data, err := ioutil.ReadFile(test.input)
			suite.Require().NoError(err)
			var documentFromFile Document
			err = json.Unmarshal(data, &documentFromFile)
			suite.Require().NoError(err)

			// compare document from chain to document from file
			suite.Assert().True(lastDoc.IsEqual(documentFromFile))
		})
	}
}

func (suite *ContractTestSuite) TestEdges() {
	tests := []struct {
		name  string
		input string
	}{
		{
			name:  "simplest",
			input: "../test/examples/simplest.json",
		},
		{
			name:  "each-type",
			input: "../test/examples/each-type.json",
		},
		{
			name:  "contribution",
			input: "../test/examples/contribution.json",
		},
	}

	doc1 := suite.createDocument(tests[0].input)
	doc2 := suite.createDocument(tests[1].input)
	doc3 := suite.createDocument(tests[2].input)

	suite.Run("test edges", func() {

		suite.createEdge(doc1.Hash, doc2.Hash, "edge1")
		edges, err := getEdges(suite.ctx, suite.api, suite.DocsContract)
		suite.Require().NoError(err)
		suite.Assert().Equal(1, len(edges))

		suite.createEdge(doc2.Hash, doc1.Hash, "edge2")
		edges, err = getEdges(suite.ctx, suite.api, suite.DocsContract)
		suite.Require().NoError(err)
		suite.Assert().Equal(2, len(edges))

		suite.createEdge(doc1.Hash, doc3.Hash, "edge3")
		edges, err = getEdges(suite.ctx, suite.api, suite.DocsContract)
		suite.Require().NoError(err)
		suite.Assert().Equal(3, len(edges))
	})
}

func TestContractSuite(t *testing.T) {
	suite.Run(t, new(ContractTestSuite))
}
