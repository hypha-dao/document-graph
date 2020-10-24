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

			lastDoc, err := CreateDocument(suite.ctx, suite.api, suite.DocsContract, suite.Accounts[1], test.input)
			suite.Require().NoError(err)

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

	doc1, err := CreateDocument(suite.ctx, suite.api, suite.DocsContract, suite.Accounts[1], "../test/examples/simplest.json")
	suite.Require().NoError(err)

	doc2, err := CreateDocument(suite.ctx, suite.api, suite.DocsContract, suite.Accounts[2], "../test/examples/each-type.json")
	suite.Require().NoError(err)

	// doc3, err := CreateDocument(suite.ctx, suite.api, suite.DocsContract, suite.Accounts[3], "../test/examples/contribution.json")
	// suite.Require().NoError(err)

	tests := []struct {
		name     string
		fromDoc  Document
		toDoc    Document
		creator  eos.AccountName
		edgeName eos.Name
	}{
		{
			name:     "Test Edge 1",
			fromDoc:  doc1,
			toDoc:    doc2,
			creator:  suite.Accounts[1],
			edgeName: "edge1",
		},
		{
			name:     "Test Edge 2",
			fromDoc:  doc2,
			toDoc:    doc1,
			creator:  suite.Accounts[2],
			edgeName: "edge2",
		},
	}

	for testIndex, test := range tests {
		suite.Run("test edges", func() {
			log.Println(test.name, "... ")

			_, err = CreateEdge(suite.ctx, suite.api, suite.DocsContract, test.creator, test.fromDoc.Hash, test.toDoc.Hash, test.edgeName)
			suite.Require().NoError(err)

			// test number of edges
			edges, err := GetEdges(suite.ctx, suite.api, suite.DocsContract)
			suite.Require().NoError(err)
			suite.Assert().Equal(testIndex+1, len(edges))

			// there should be 1 edge from doc1 to doc2, named edgeName
			edgesFrom, err := test.fromDoc.GetEdgesFrom(suite.ctx, suite.api, suite.DocsContract)
			suite.Require().NoError(err)
			suite.Assert().Equal(1, len(edgesFrom))
			suite.Assert().Equal(edgesFrom[0].EdgeName, test.edgeName)
			suite.Assert().Equal(edgesFrom[0].FromNode, test.fromDoc.Hash)
			suite.Assert().Equal(edgesFrom[0].ToNode, test.toDoc.Hash)

			// there should be 0 edges from doc2 to doc1
			edgesTo, err := test.toDoc.GetEdgesTo(suite.ctx, suite.api, suite.DocsContract)
			suite.Require().NoError(err)
			suite.Assert().Equal(1, len(edgesTo))
			suite.Assert().Equal(edgesTo[0].EdgeName, test.edgeName)
			suite.Assert().Equal(edgesTo[0].FromNode, test.fromDoc.Hash)
			suite.Assert().Equal(edgesTo[0].ToNode, test.toDoc.Hash)

			// there should be 1 edge from doc1 to doc2, named edgeName
			edgesFromByName, err := test.fromDoc.GetEdgesFromByName(suite.ctx, suite.api, suite.DocsContract, test.edgeName)
			suite.Require().NoError(err)
			suite.Assert().Equal(1, len(edgesFromByName))
			suite.Assert().Equal(edgesFromByName[0].EdgeName, test.edgeName)
			suite.Assert().Equal(edgesFromByName[0].FromNode, test.fromDoc.Hash)
			suite.Assert().Equal(edgesFromByName[0].ToNode, test.toDoc.Hash)

			// there should be 1 edge from doc1 to doc2, named edgeName
			edgesToByName, err := test.toDoc.GetEdgesToByName(suite.ctx, suite.api, suite.DocsContract, test.edgeName)
			suite.Require().NoError(err)
			suite.Assert().Equal(1, len(edgesToByName))
			suite.Assert().Equal(edgesToByName[0].EdgeName, test.edgeName)
			suite.Assert().Equal(edgesToByName[0].FromNode, test.fromDoc.Hash)
			suite.Assert().Equal(edgesToByName[0].ToNode, test.toDoc.Hash)

			// there should be 0 edge from doc1 to doc2, named wrongedge
			edgesFromByName, err = test.fromDoc.GetEdgesFromByName(suite.ctx, suite.api, suite.DocsContract, eos.Name("wrongedge"))
			suite.Require().NoError(err)
			suite.Assert().Equal(0, len(edgesFromByName))

			// there should be 0 edge from doc1 to doc2, named edgeName
			edgesToByName, err = test.toDoc.GetEdgesToByName(suite.ctx, suite.api, suite.DocsContract, eos.Name("wrongedge"))
			suite.Require().NoError(err)
			suite.Assert().Equal(0, len(edgesToByName))
		})
	}
}

func TestContractSuite(t *testing.T) {
	suite.Run(t, new(ContractTestSuite))
}
