package docgraph_test

import (
	"context"
	"strconv"
	"testing"

	eostest "github.com/digital-scarcity/eos-go-test"

	eos "github.com/eoscanada/eos-go"
	"gotest.tools/assert"
)

type Environment struct {
	ctx context.Context
	api eos.API

	Docs     eos.AccountName
	Creators []eos.AccountName
}

func SetupEnvironment(t *testing.T) *Environment {

	var env Environment
	env.api = *eos.New(testingEndpoint)
	// api.Debug = true
	env.ctx = context.Background()

	keyBag := &eos.KeyBag{}
	err := keyBag.ImportPrivateKey(env.ctx, eostest.DefaultKey())
	assert.NilError(t, err)

	env.api.SetSigner(keyBag)

	env.Docs, err = eostest.CreateAccountFromString(env.ctx, &env.api, "documents", eostest.DefaultKey())
	assert.NilError(t, err)

	_, err = eostest.SetContract(env.ctx, &env.api, env.Docs, "../build/docs/docs.wasm", "../build/docs/docs.abi")
	assert.NilError(t, err)

	for i := 1; i < 5; i++ {

		creator, err := eostest.CreateAccountFromString(env.ctx, &env.api, "creator"+strconv.Itoa(i), eostest.DefaultKey())
		assert.NilError(t, err)

		env.Creators = append(env.Creators, creator)
	}
	return &env
}
