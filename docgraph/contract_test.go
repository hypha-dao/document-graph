package docgraph_test

import (
	"encoding/json"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"testing"
	"time"

	eos "github.com/eoscanada/eos-go"
	"github.com/hypha-dao/document/docgraph"
	"gotest.tools/assert"
)

const testingEndpoint = "http://localhost:8888"

var env *Environment

func setupTestCase(t *testing.T) func(t *testing.T) {
	t.Log("Bootstrapping testing environment ...")

	_, err := exec.Command("sh", "-c", "pkill -SIGINT nodeos").Output()
	if err == nil {
		pause(t, time.Second, "Killing nodeos ...", "")
	}

	t.Log("Starting nodeos from 'nodeos.sh' script ...")
	cmd := exec.Command("./nodeos.sh")
	cmd.Stdout = os.Stdout
	err = cmd.Start()
	assert.NilError(t, err)

	t.Log("nodeos PID: ", cmd.Process.Pid)

	pause(t, time.Second, "", "")

	return func(t *testing.T) {
		folderName := "test_results"
		t.Log("Saving graph to : ", folderName)
		err := SaveGraph(env.ctx, &env.api, env.Docs, folderName)
		assert.NilError(t, err)
	}
}

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

func TestEdges(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)
	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	doc1, err := docgraph.CreateDocument(env.ctx, &env.api, env.Docs, env.Creators[0], "../test/examples/simplest.json")
	assert.NilError(t, err)

	doc2, err := docgraph.CreateDocument(env.ctx, &env.api, env.Docs, env.Creators[0], "../test/examples/each-type.json")
	assert.NilError(t, err)

	// doc3, err := CreateDocument(env.ctx, &env.api, env.Docs, suite.Accounts[3], "../test/examples/contribution.json")
	// suite.Require().NoError(err)

	tests := []struct {
		name     string
		fromDoc  docgraph.Document
		toDoc    docgraph.Document
		creator  eos.AccountName
		edgeName eos.Name
	}{
		{
			name:     "Test Edge 1",
			fromDoc:  doc1,
			toDoc:    doc2,
			creator:  env.Creators[1],
			edgeName: "edge1",
		},
		{
			name:     "Test Edge 2",
			fromDoc:  doc2,
			toDoc:    doc1,
			creator:  env.Creators[2],
			edgeName: "edge2",
		},
	}

	for testIndex, test := range tests {
		t.Run("test edges", func(t *testing.T) {
			log.Println(test.name, "... ")

			_, err = docgraph.CreateEdge(env.ctx, &env.api, env.Docs, env.Creators[0], test.fromDoc.Hash, test.toDoc.Hash, test.edgeName)
			assert.NilError(t, err)

			// test number of edges
			edges, err := GetAllEdges(env.ctx, &env.api, env.Docs)
			assert.NilError(t, err)
			assert.Equal(t, testIndex+1, len(edges))

			// there should be 1 edge from doc1 to doc2, named edgeName
			edgesFrom, err := test.fromDoc.GetEdgesFrom(env.ctx, &env.api, env.Docs)
			assert.NilError(t, err)
			assert.Equal(t, 1, len(edgesFrom))
			assert.Equal(t, edgesFrom[0].EdgeName, test.edgeName)
			assert.Equal(t, edgesFrom[0].FromNode.String(), test.fromDoc.Hash.String())
			assert.Equal(t, edgesFrom[0].ToNode.String(), test.toDoc.Hash.String())

			// there should be 0 edges from doc2 to doc1
			edgesTo, err := test.toDoc.GetEdgesTo(env.ctx, &env.api, env.Docs)
			assert.NilError(t, err)
			assert.Equal(t, 1, len(edgesTo))
			assert.Equal(t, edgesTo[0].EdgeName, test.edgeName)
			assert.Equal(t, edgesTo[0].FromNode.String(), test.fromDoc.Hash.String())
			assert.Equal(t, edgesTo[0].ToNode.String(), test.toDoc.Hash.String())

			// there should be 1 edge from doc1 to doc2, named edgeName
			edgesFromByName, err := test.fromDoc.GetEdgesFromByName(env.ctx, &env.api, env.Docs, test.edgeName)
			assert.NilError(t, err)
			assert.Equal(t, 1, len(edgesFromByName))
			assert.Equal(t, edgesFromByName[0].EdgeName, test.edgeName)
			assert.Equal(t, edgesFromByName[0].FromNode.String(), test.fromDoc.Hash.String())
			assert.Equal(t, edgesFromByName[0].ToNode.String(), test.toDoc.Hash.String())

			// there should be 1 edge from doc1 to doc2, named edgeName
			edgesToByName, err := test.toDoc.GetEdgesToByName(env.ctx, &env.api, env.Docs, test.edgeName)
			assert.NilError(t, err)
			assert.Equal(t, 1, len(edgesToByName))
			assert.Equal(t, edgesToByName[0].EdgeName, test.edgeName)
			assert.Equal(t, edgesToByName[0].FromNode.String(), test.fromDoc.Hash.String())
			assert.Equal(t, edgesToByName[0].ToNode.String(), test.toDoc.Hash.String())

			// there should be 0 edge from doc1 to doc2, named wrongedge
			edgesFromByName, err = test.fromDoc.GetEdgesFromByName(env.ctx, &env.api, env.Docs, eos.Name("wrongedge"))
			assert.NilError(t, err)
			assert.Equal(t, 0, len(edgesFromByName))

			// there should be 0 edge from doc1 to doc2, named edgeName
			edgesToByName, err = test.toDoc.GetEdgesToByName(env.ctx, &env.api, env.Docs, eos.Name("wrongedge"))
			assert.NilError(t, err)
			assert.Equal(t, 0, len(edgesToByName))

			doesExist, err := docgraph.EdgeExists(env.ctx, &env.api, env.Docs, test.fromDoc, test.toDoc, test.edgeName)
			assert.NilError(t, err)
			assert.Assert(t, doesExist)

			doesNotExist, err := docgraph.EdgeExists(env.ctx, &env.api, env.Docs, test.fromDoc, test.toDoc, eos.Name("doesnotexist"))
			assert.NilError(t, err)
			assert.Assert(t, !doesNotExist)
		})
	}
}

func TestRemoveEdges(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)

	// var env Environment
	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	// var docs []Document
	var err error
	docs := make([]docgraph.Document, 10)
	for i := 1; i < 10; i++ {
		docs[i], err = CreateRandomDocument(env.ctx, &env.api, env.Docs, env.Creators[1])
		assert.NilError(t, err)
	}

	for i := 1; i < 5; i++ {
		_, err = docgraph.CreateEdge(env.ctx, &env.api, env.Docs, env.Creators[1], docs[i].Hash, docs[i+1].Hash, "test")
		assert.NilError(t, err)
	}
}

func TestLoadDocument(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)

	// var env Environment
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
