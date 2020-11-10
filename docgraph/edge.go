package docgraph

import (
	"context"

	eostest "github.com/digital-scarcity/eos-go-test"
	eos "github.com/eoscanada/eos-go"
)

// Edge is a directional, named connection from one graph to another
type Edge struct {
	ID          uint64             `json:"id"`
	FromNode    eos.Checksum256    `json:"from_node"`
	ToNode      eos.Checksum256    `json:"to_node"`
	EdgeName    eos.Name           `json:"edge_name"`
	CreatedDate eos.BlockTimestamp `json:"created_date"`
}

type removeEdges struct {
	FromNode eos.Checksum256 `json:"from_node"`
	EdgeName eos.Name        `json:"edge_name"`
	Strict   bool            `json:"strict"`
}

// RemoveEdges ...
func RemoveEdges(ctx context.Context, api *eos.API,
	contract eos.AccountName,
	fromHash eos.Checksum256, edgeName eos.Name) (string, error) {

	actions := []*eos.Action{{
		Account: contract,
		Name:    eos.ActN("removeedgese"),
		Authorization: []eos.PermissionLevel{
			{Actor: contract, Permission: eos.PN("active")},
		},
		ActionData: eos.NewActionData(removeEdges{
			FromNode: fromHash,
			EdgeName: edgeName,
			Strict:   true,
		}),
	}}
	return eostest.ExecTrx(ctx, api, actions)
}
