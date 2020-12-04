package docgraph_test

import (
	"encoding/json"
	"io/ioutil"
	"testing"

	"github.com/eoscanada/eos-go"

	"github.com/hypha-dao/document-graph/docgraph"
	"gotest.tools/v3/assert"
)

func TestDocuments(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)

	// var env Environment
	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	t.Run("Test Documents", func(t *testing.T) {

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
			t.Run(test.name, func(t *testing.T) {

				lastDoc, err := docgraph.CreateDocument(env.ctx, &env.api, env.Docs, env.Creators[0], test.input)
				assert.NilError(t, err)

				// unmarshal JSON into a Document
				data, err := ioutil.ReadFile(test.input)
				assert.NilError(t, err)
				var documentFromFile docgraph.Document
				err = json.Unmarshal(data, &documentFromFile)
				assert.NilError(t, err)

				// compare document from chain to document from file
				assert.Assert(t, lastDoc.IsEqual(documentFromFile))
			})
		}
	})
}

func TestLoadDocument(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)

	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	doc, err := docgraph.CreateDocument(env.ctx, &env.api, env.Docs, env.Creators[1], "../test/examples/simplest.json")
	assert.NilError(t, err)

	loadedDoc, err := docgraph.LoadDocument(env.ctx, &env.api, env.Docs, doc.Hash.String())
	assert.NilError(t, err)
	assert.Equal(t, doc.Hash.String(), loadedDoc.Hash.String())
	assert.Equal(t, doc.Creator, loadedDoc.Creator)

	_, err = docgraph.LoadDocument(env.ctx, &env.api, env.Docs, "ahashthatwillnotexist")
	assert.ErrorContains(t, err, "Internal Service Error")
}

func TestEraseDocument(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)

	// var env Environment
	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	randomDoc, err := CreateRandomDocument(env.ctx, &env.api, env.Docs, env.Creators[1])
	assert.NilError(t, err)

	_, err = docgraph.EraseDocument(env.ctx, &env.api, env.Docs, randomDoc.Hash)
	assert.NilError(t, err)

	_, err = docgraph.LoadDocument(env.ctx, &env.api, env.Docs, randomDoc.Hash.String())
	assert.ErrorContains(t, err, "document not found")
}

func TestCreateRoot(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)

	// var env Environment
	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	rootDoc, err := CreateRoot(env.ctx, &env.api, env.Docs, env.Docs)
	assert.NilError(t, err)

	t.Log("Root Document hash: ", rootDoc.Hash.String())
}

func TestGetContent(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)

	// var env Environment
	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	t.Run("Test Get Content", func(t *testing.T) {

		tests := []struct {
			name  string
			input string
		}{
			{
				name:  "each-type",
				input: "../test/examples/each-type.json",
			},
		}

		for _, test := range tests {
			t.Run(test.name, func(t *testing.T) {

				lastDoc, err := docgraph.CreateDocument(env.ctx, &env.api, env.Docs, env.Creators[0], test.input)
				assert.NilError(t, err)

				// unmarshal JSON into a Document
				data, err := ioutil.ReadFile(test.input)
				assert.NilError(t, err)
				var documentFromFile docgraph.Document
				err = json.Unmarshal(data, &documentFromFile)
				assert.NilError(t, err)

				// compare document from chain to document from file
				assert.Assert(t, lastDoc.IsEqual(documentFromFile))

				salary, _ := eos.NewAssetFromString("130.00 USD")
				_, err = GetAssetTest(env.ctx, &env.api, env.Docs, lastDoc, "My Content Group Label", "salary_amount", salary)
				assert.NilError(t, err)

				wrongSalary, _ := eos.NewAssetFromString("131.00 USD")
				_, err = GetAssetTest(env.ctx, &env.api, env.Docs, lastDoc, "My Content Group Label", "salary_amount", wrongSalary)
				assert.ErrorContains(t, err, "read value does not equal content value")

				_, err = GetAssetTest(env.ctx, &env.api, env.Docs, lastDoc, "My Content Group Label", "wrong_content_label", salary)
				assert.ErrorContains(t, err, "contentGroup or contentLabel does not exist")

				_, err = GetAssetTest(env.ctx, &env.api, env.Docs, lastDoc, "Nonexistent Content Group Label", "salary_amount", salary)
				assert.ErrorContains(t, err, "contentGroup or contentLabel does not exist")
			})
		}
	})
}

func TestGetGroup(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)

	// var env Environment
	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	t.Run("Test Get Content", func(t *testing.T) {

		tests := []struct {
			name  string
			input string
		}{
			{
				name:  "each-type",
				input: "../test/examples/each-type.json",
			},
		}

		for _, test := range tests {
			t.Run(test.name, func(t *testing.T) {

				lastDoc, err := docgraph.CreateDocument(env.ctx, &env.api, env.Docs, env.Creators[0], test.input)
				assert.NilError(t, err)

				// unmarshal JSON into a Document
				data, err := ioutil.ReadFile(test.input)
				assert.NilError(t, err)
				var documentFromFile docgraph.Document
				err = json.Unmarshal(data, &documentFromFile)
				assert.NilError(t, err)

				// compare document from chain to document from file
				assert.Assert(t, lastDoc.IsEqual(documentFromFile))

				_, err = GetGroupTest(env.ctx, &env.api, env.Docs, lastDoc, "My Content Group Label")
				assert.NilError(t, err)

				_, err = GetGroupTest(env.ctx, &env.api, env.Docs, lastDoc, "Nonexistent Content Group Label")
				assert.ErrorContains(t, err, "group was not found")

			})
		}
	})
}
