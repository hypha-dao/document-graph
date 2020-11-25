package docgraph

import (
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

// RemoveEdges ...
type RemoveEdges struct {
	FromNode eos.Checksum256 `json:"from_node"`
	EdgeName eos.Name        `json:"edge_name"`
	Strict   bool            `json:"strict"`
}

type removeEdgesFT struct {
	FromNode eos.Checksum256 `json:"from_node"`
	ToNode   eos.Checksum256 `json:"to_node"`
	Strict   bool            `json:"strict"`
}

// RemoveEdgesFromAndName ...
// func RemoveEdgesFromAndName(ctx context.Context, api *eos.API,
// 	contract eos.AccountName,
// 	fromHash eos.Checksum256, edgeName eos.Name) (string, error) {

// 	actions := []*eos.Action{{
// 		Account: contract,
// 		Name:    eos.ActN("remedgesfn"),
// 		Authorization: []eos.PermissionLevel{
// 			{Actor: contract, Permission: eos.PN("active")},
// 		},
// 		ActionData: eos.NewActionData(RemoveEdges{
// 			FromNode: fromHash,
// 			EdgeName: edgeName,
// 			Strict:   true,
// 		}),
// 	}}
// 	return eostest.ExecTrx(ctx, api, actions)
// }

// RemoveEdgesFromAndTo ...
// func RemoveEdgesFromAndTo(ctx context.Context, api *eos.API,
// 	contract eos.AccountName,
// 	fromHash, toHash eos.Checksum256) (string, error) {

// 	actions := []*eos.Action{{
// 		Account: contract,
// 		Name:    eos.ActN("remedgesft"),
// 		Authorization: []eos.PermissionLevel{
// 			{Actor: contract, Permission: eos.PN("active")},
// 		},
// 		ActionData: eos.NewActionData(removeEdgesFT{
// 			FromNode: fromHash,
// 			ToNode:   toHash,
// 			Strict:   true,
// 		}),
// 	}}
// 	return eostest.ExecTrx(ctx, api, actions)
// }
