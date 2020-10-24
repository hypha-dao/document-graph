package docgraph

import eos "github.com/eoscanada/eos-go"

// Edge is a directional, named connection from one graph to another
type Edge struct {
	ID          uint64             `json:"id"`
	FromNode    eos.Checksum256    `json:"from_node"`
	ToNode      eos.Checksum256    `json:"to_node"`
	EdgeName    eos.Name           `json:"edge_name"`
	CreatedDate eos.BlockTimestamp `json:"created_date"`
}
