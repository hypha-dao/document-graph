package docgraph_test

import (
	"fmt"
	"os"
	"os/exec"
	"testing"
	"time"

	eos "github.com/eoscanada/eos-go"
	"github.com/hypha-dao/document-graph/docgraph"
	"gotest.tools/v3/assert"
)

const testingEndpoint = "http://localhost:8888"

var env *Environment
var chainResponsePause time.Duration

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
	chainResponsePause = time.Second

	t.Log("nodeos PID: ", cmd.Process.Pid)

	pause(t, 500*time.Millisecond, "", "")

	return func(t *testing.T) {
		folderName := "test_results"
		t.Log("Saving graph to : ", folderName)
		err := SaveGraph(env.ctx, &env.api, env.Docs, folderName)
		assert.NilError(t, err)
	}
}

func TestGetOrNewNew(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)

	// var env Environment
	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	_, err := CreateRandomDocument(env.ctx, &env.api, env.Docs, env.Creators[1])
	assert.NilError(t, err)

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
	var randomDoc docgraph.Document
	randomDoc.ContentGroups = cgs

	// should be a legit new document
	_, err = GetOrNewNew(env.ctx, &env.api, env.Docs, env.Creators[1], randomDoc)
	assert.NilError(t, err)
}

func TestGetOrNewGet(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)

	// var env Environment
	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	randomDoc, err := CreateRandomDocument(env.ctx, &env.api, env.Docs, env.Creators[1])
	assert.NilError(t, err)

	// should NOT be a legit new document
	sameRandomDoc, err := GetOrNewGet(env.ctx, &env.api, env.Docs, env.Creators[1], randomDoc)
	assert.NilError(t, err)

	assert.Equal(t, randomDoc.Hash.String(), sameRandomDoc.Hash.String())
}

func TestGetLastDocOfEdgeName(t *testing.T) {

	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)

	// var env Environment
	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	randomDoc1, err := CreateRandomDocument(env.ctx, &env.api, env.Docs, env.Creators[1])
	assert.NilError(t, err)

	randomDoc2, err := CreateRandomDocument(env.ctx, &env.api, env.Docs, env.Creators[1])
	assert.NilError(t, err)

	_, err = docgraph.CreateEdge(env.ctx, &env.api, env.Docs, env.Creators[1], randomDoc1.Hash, randomDoc2.Hash, "testlastedge")
	assert.NilError(t, err)

	lastDocument, err := docgraph.GetLastDocumentOfEdge(env.ctx, &env.api, env.Docs, "testlastedge")
	assert.NilError(t, err)
	assert.Equal(t, randomDoc2.Hash.String(), lastDocument.Hash.String())
}

func TestManyDocuments(t *testing.T) {
	teardownTestCase := setupTestCase(t)
	defer teardownTestCase(t)

	env = SetupEnvironment(t)
	t.Log("\nEnvironment Setup complete\n")

	for i := 1; i < 1000; i++ {
		_, err := CreateRandomDocument(env.ctx, &env.api, env.Docs, env.Creators[1])
		assert.NilError(t, err)
	}

	docs, err := docgraph.GetAllDocuments(env.ctx, &env.api, env.Docs)
	if err != nil {
		panic(fmt.Errorf("cannot get all documents: %v", err))
	}

	assert.NilError(t, err)
	assert.Equal(t, len(docs), 999)
}
