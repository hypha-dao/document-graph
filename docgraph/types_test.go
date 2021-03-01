package docgraph

import (
	"encoding/json"
	"io/ioutil"
	"testing"

	eos "github.com/eoscanada/eos-go"
	"github.com/stretchr/testify/require"
	"gotest.tools/assert"
)

const testDocument = `{
	"id": 24,
	"hash": "05e81010c4600ed5d978d2ddf22420ffdf6c4094f4b3822711f0596c7c342ccb",
	"creator": "johnnyhypha1",
	"content_groups": [[{
		  "label": "content_group_label",
		  "value": [
			"string",
			"details"
		  ]
		},{
		  "label": "title",
		  "value": [
			"string",
			"Healer"
		  ]
		},{
		  "label": "description",
		  "value": [
			"string",
			"Holder of indigenous wisdom ready to transfer the knowledge to others willing to receive"
		  ]
		},{
		  "label": "icon",
		  "value": [
			"string",
			"https://myiconlink.com/fakelink"
		  ]
		},{
		  "label": "seeds_coefficient_x10000",
		  "value": [
			"int64",
			10010
		  ]
		},{
		  "label": "hypha_coefficient_x10000",
		  "value": [
			"int64",
			10015
		  ]
		},{
		  "label": "hvoice_coefficient_x10000",
		  "value": [
			"int64",
			10000
		  ]
		},{
		  "label": "husd_coefficient_x10000",
		  "value": [
			"int64",
			10100
		  ]
		}
	  ],[{
		  "label": "content_group_label",
		  "value": [
			"string",
			"system"
		  ]
		},{
		  "label": "client_version",
		  "value": [
			"string",
			"1.0.13 0c81dde6"
		  ]
		},{
		  "label": "contract_version",
		  "value": [
			"string",
			"1.0.1 366e8dfe"
		  ]
		},{
		  "label": "ballot_id",
		  "value": [
			"name",
			"hypha1....14i"
		  ]
		},{
		  "label": "proposer",
		  "value": [
			"name",
			"johnnyhypha1"
		  ]
		},{
		  "label": "type",
		  "value": [
			"name",
			"badge"
		  ]
		}
	  ]
	],
	"certificates": [
		{
			"certifier": "dao.hypha",
			"notes": "certification notes",
			"certification_date": "2020-10-19T14:02:32.500"
		}
	],
	"created_date": "2020-10-16T14:02:32.500"
  }`

func TestEdgeJSONUnmarshal(t *testing.T) {
	input := `{
		"id": 349057277,
		"from_node": "7463fa7dda551b9c4bbd2ba17b793931c825cefff9eede14461fd1a5c9f07d15",
		"to_node": "d4ec74355830056924c83f20ffb1a22ad0c5145a96daddf6301897a092de951e",
		"edge_name": "memberof",
		"created_date": "2020-10-16T14:11:37.000"
	  }`

	var e Edge
	err := json.Unmarshal([]byte(input), &e)

	require.NoError(t, err)
	assert.Equal(t, e.ID, uint64(349057277), "id")
	assert.Equal(t, e.EdgeName, eos.Name("memberof"), "edge_name")
	assert.Equal(t, e.FromNode.String(), string("7463fa7dda551b9c4bbd2ba17b793931c825cefff9eede14461fd1a5c9f07d15"), "from_node")
	assert.Equal(t, e.ToNode.String(), string("d4ec74355830056924c83f20ffb1a22ad0c5145a96daddf6301897a092de951e"), "to_node")
}

func TestDocumentJSONUnmarshal(t *testing.T) {
	var d Document

	err := json.Unmarshal([]byte(testDocument), &d)

	require.NoError(t, err)
	assert.Equal(t, d.ID, uint64(24), "id")
	assert.Equal(t, d.Hash.String(), string("05e81010c4600ed5d978d2ddf22420ffdf6c4094f4b3822711f0596c7c342ccb"), "hash")
	assert.Equal(t, d.Creator, eos.AN("johnnyhypha1"), "creator")
	assert.Equal(t, len(d.ContentGroups), 2, "content groups length")
	assert.Equal(t, len(d.ContentGroups[0]), 8, "first content group length")
	assert.Equal(t, d.ContentGroups[0][1].Label, string("title"), "title label")
	assert.Equal(t, d.ContentGroups[0][1].Value.Impl, string("Healer"), "title value")
}

func TestGetContent(t *testing.T) {
	var d Document

	err := json.Unmarshal([]byte(testDocument), &d)

	require.NoError(t, err)

	content, err := d.GetContent("title")
	require.NoError(t, err)
	assert.Equal(t, content.Impl, string("Healer"), "get title content")

	content, err = d.GetContent("husd_coefficient_x10000")
	require.NoError(t, err)
	assert.Equal(t, content.Impl, int64(10100), "husd coefficient get content")
}

func TestGetContentNotFound(t *testing.T) {
	var d Document

	err := json.Unmarshal([]byte(testDocument), &d)

	require.NoError(t, err)

	content, err := d.GetContent("label not found")
	require.Error(t, &ContentNotFoundError{
		Label:        "label not found",
		DocumentHash: d.Hash,
	})

	require.Nil(t, content)
}

func TestAddContent(t *testing.T) {
	var d Document

	err := json.Unmarshal([]byte(testDocument), &d)
	require.NoError(t, err)

	require.Equal(t, 8, len(d.ContentGroups[0]))

	hash := eos.Checksum256("7463fa7dda551b9c4bbd2ba17b793931c825cefff9eede14461fd1a5c9f07d15")

	fv := &FlexValue{
		BaseVariant: eos.BaseVariant{
			TypeID: FlexValueVariant.TypeID("checksum256"),
			Impl:   hash,
		}}

	var ci ContentItem
	ci.Label = "badge"
	ci.Value = fv

	d.ContentGroups[0] = append(d.ContentGroups[0], ci)
	require.Equal(t, 9, len(d.ContentGroups[0]))
}

func TestExamplePayloads(t *testing.T) {
	// var d Document
	var testFile Document

	tests := []struct {
		name  string
		input string
	}{
		{
			name:  "simplest",
			input: "examples/simplest.json",
		},
		{
			name:  "each-type",
			input: "examples/each-type.json",
		},
		{
			name:  "contribution",
			input: "examples/contribution.json",
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {

			data, err := ioutil.ReadFile(test.input)
			require.NoError(t, err)
			// log.Println(string(data))

			err = json.Unmarshal(data, &testFile)
			require.NoError(t, err)
		})
	}
}

func TestDocumentEquality(t *testing.T) {
	var d1, d2 Document

	tests := []struct {
		name  string
		input string
	}{
		{
			name:  "simplest",
			input: "examples/simplest.json",
		},
		{
			name:  "each-type",
			input: "examples/each-type.json",
		},
		{
			name:  "contribution",
			input: "examples/contribution.json",
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {

			data, err := ioutil.ReadFile(test.input)
			require.NoError(t, err)
			// log.Println(string(data))

			err = json.Unmarshal(data, &d1)
			require.NoError(t, err)

			err = json.Unmarshal(data, &d2)
			require.NoError(t, err)

			require.True(t, d1.IsEqual(d2), "documents are not equal")
		})
	}
}

func TestDocumentInEquality(t *testing.T) {
	var d1, d2 Document

	tests := []struct {
		name   string
		input1 string
		input2 string
	}{
		{
			name:   "simplest vs each-type",
			input1: "examples/simplest.json",
			input2: "examples/each-type.json",
		},
		{
			name:   "each-type vs contribution",
			input1: "examples/each-type.json",
			input2: "examples/contribution.json",
		},
		{
			name:   "contribution vs simplest",
			input1: "examples/contribution.json",
			input2: "examples/simplest.json",
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {

			data1, err := ioutil.ReadFile(test.input1)
			require.NoError(t, err)

			err = json.Unmarshal(data1, &d1)
			require.NoError(t, err)

			data2, err := ioutil.ReadFile(test.input2)
			require.NoError(t, err)

			err = json.Unmarshal(data2, &d2)
			require.NoError(t, err)

			require.False(t, d1.IsEqual(d2), "documents are equal")
		})
	}
}
