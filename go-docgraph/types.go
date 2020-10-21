package docgraph

import (
	"fmt"

	eos "github.com/eoscanada/eos-go"
)

// flexValueVariant may hold a name, int64, asset, string, or time_point
var flexValueVariant = eos.NewVariantDefinition([]eos.VariantType{
	{Name: "name", Type: eos.Name("")},
	{Name: "int64", Type: int64(0)},
	{Name: "asset", Type: (*eos.Asset)(nil)}, // Syntax for pointer to a type, could be any struct
	{Name: "string", Type: ""},
	{Name: "time_point", Type: eos.TimePoint(0)},
	{Name: "checksum256", Type: eos.Checksum256([]byte("0"))},
})

// FlexValue may hold any of the common EOSIO types
// name, int64, asset, string, time_point, or checksum256
type FlexValue struct {
	eos.BaseVariant
}

// MarshalJSON translates to []byte
func (a *FlexValue) MarshalJSON() ([]byte, error) {
	return a.BaseVariant.MarshalJSON(flexValueVariant)
}

// UnmarshalJSON translates flexValueVariant
func (a *FlexValue) UnmarshalJSON(data []byte) error {
	return a.BaseVariant.UnmarshalJSON(data, flexValueVariant)
}

// UnmarshalBinary ...
func (a *FlexValue) UnmarshalBinary(decoder *eos.Decoder) error {
	return a.BaseVariant.UnmarshalBinaryVariant(decoder, flexValueVariant)
}

// ContentNotFoundError is used when content matching
// a specific label is requested but not found in a document
type ContentNotFoundError struct {
	Label        string
	DocumentHash eos.Checksum256
}

func (c *ContentNotFoundError) Error() string {
	return fmt.Sprintf("content label not found: %v in document: %v", c.Label, c.DocumentHash.String())
}

// ContentItem ...
type ContentItem struct {
	Label string     `json:"label"`
	Value *FlexValue `json:"value"`
}

// ContentGroup ...
type ContentGroup []ContentItem

// CGFileInput ...
// type CGFileInput struct {
// 	ContentItems [][]ContentItem
// }

// Document is a node in the document graph
// A document may hold any arbitrary, EOSIO compatible data
type Document struct {
	ID            uint64          `json:"id"`
	Hash          eos.Checksum256 `json:"hash"`
	Creator       eos.Name        `json:"creator"`
	ContentGroups []ContentGroup  `json:"content_groups"`
	Certificates  []struct {
		Certifier         eos.Name           `json:"certifier"`
		Notes             string             `json:"notes"`
		CertificationDate eos.BlockTimestamp `json:"certification_date"`
	} `json:"certificates"`
	CreatedDate eos.BlockTimestamp `json:"created_date"`
}

// GetContent returns a FlexValue of the content with the matching label
// or an instance of ContentNotFoundError
func (d *Document) GetContent(label string) (*FlexValue, error) {
	for _, contentGroup := range d.ContentGroups {
		for _, content := range contentGroup {
			if content.Label == label {
				return content.Value, nil
			}
		}
	}
	return nil, &ContentNotFoundError{
		Label:        label,
		DocumentHash: d.Hash,
	}
}

// Edge is a directional, named connection from one graph to another
type Edge struct {
	ID          uint64             `json:"id"`
	FromNode    eos.Checksum256    `json:"from_node"`
	ToNode      eos.Checksum256    `json:"to_node"`
	EdgeName    eos.Name           `json:"edge_name"`
	CreatedDate eos.BlockTimestamp `json:"created_date"`
}
