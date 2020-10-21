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

func (suite *ContractTestSuite) TestSimplest() {

	// var d Document
	data, err := ioutil.ReadFile("../test/examples/simplest.json")
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

	simplestValue, err := lastDoc.GetContent("simplest_label")
	suite.Assert().Equal(simplestValue.Impl, string("Simplest"))

	// TODO: deep compare of data from file and data from chain
}

func TestContractSuite(t *testing.T) {
	suite.Run(t, new(ContractTestSuite))
}
