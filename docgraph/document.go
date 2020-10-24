package docgraph

import (
	"fmt"
	"log"
	"strings"

	eos "github.com/eoscanada/eos-go"
)

// FlexValueVariant may hold a name, int64, asset, string, or time_point
var FlexValueVariant = eos.NewVariantDefinition([]eos.VariantType{
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

func (fv *FlexValue) String() string {
	switch v := fv.Impl.(type) {
	case eos.Name:
		return string(v)
	case int64:
		return string(v)
	case *eos.Asset:
		return v.String()
	case string:
		return v
	case eos.TimePoint:
		return v.String()
	case eos.Checksum256:
		return v.String()
	default:
		panic(fmt.Errorf("received an unexpected type %T for metadata variant %T", v, fv))
	}
}

// IsEqual evaluates if the two FlexValues have the same types and values (deep compare)
func (fv *FlexValue) IsEqual(fv2 *FlexValue) bool {

	if fv.TypeID != fv2.TypeID {
		log.Println("FlexValue types inequal: ", fv.TypeID, " vs ", fv2.TypeID)
		return false
	}

	if fv.String() != fv2.String() {
		log.Println("FlexValue Values.String() inequal: ", fv.String(), " vs ", fv2.String())
		return false
	}

	return true
}

// MarshalJSON translates to []byte
func (fv *FlexValue) MarshalJSON() ([]byte, error) {
	return fv.BaseVariant.MarshalJSON(flexValueVariant)
}

// UnmarshalJSON translates flexValueVariant
func (fv *FlexValue) UnmarshalJSON(data []byte) error {
	return fv.BaseVariant.UnmarshalJSON(data, flexValueVariant)
}

// UnmarshalBinary ...
func (fv *FlexValue) UnmarshalBinary(decoder *eos.Decoder) error {
	return fv.BaseVariant.UnmarshalBinaryVariant(decoder, flexValueVariant)
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

// IsEqual evalutes if the label, value type and value impl are the same
func (c *ContentItem) IsEqual(c2 ContentItem) bool {
	if strings.Compare(c.Label, c2.Label) != 0 {
		log.Println("ContentItem labels inequal: ", c.Label, " vs ", c2.Label)
		return false
	}

	if !c.Value.IsEqual(c2.Value) {
		log.Println("ContentItems inequal: ", c.Value, " vs ", c2.Value)
		return false
	}

	return true
}

// ContentGroup ...
type ContentGroup []ContentItem

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

// IsEqual is a deep equal comparison of two documents
//
func (d *Document) IsEqual(d2 Document) bool {

	// ensure the same number of content groups
	if len(d.ContentGroups) != len(d2.ContentGroups) {
		log.Println("ContentGroups lengths inequal: ", len(d.ContentGroups), " vs ", len(d2.ContentGroups))
		return false
	}

	for contentGroupIndex, contentGroup1 := range d.ContentGroups {
		contentGroup2 := d2.ContentGroups[contentGroupIndex]

		// ensure these two content groups have the same number of items
		if len(contentGroup1) != len(contentGroup2) {
			log.Println("ContentGroup lengths inequal for CG index: ", contentGroupIndex, "; ", len(contentGroup1), " vs ", len(contentGroup2))
			return false
		}

		for contentIndex, content1 := range contentGroup1 {
			content2 := contentGroup2[contentIndex]

			// ensure these content items have the same label and same value
			if !content1.IsEqual(content2) {
				return false
			}
		}
	}

	// if we got through all the above checks, the documents are equal
	return true
}
