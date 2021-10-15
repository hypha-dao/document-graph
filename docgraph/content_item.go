package docgraph

import (
	"fmt"
	"log"
	"strings"

	eos "github.com/eoscanada/eos-go"
)

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

// InvalidTypeError is used the type of a FlexValue doesn't match expectations
type InvalidTypeError struct {
	Label        string
	ExpectedType string
	FlexValue    *FlexValue
}

func (c *InvalidTypeError) Error() string {
	return fmt.Sprintf("received an unexpected type %T for metadata variant %T", c.ExpectedType, c.FlexValue)
}

// FlexValueVariant may hold a name, int64, asset, string, or time_point
var FlexValueVariant = eos.NewVariantDefinition([]eos.VariantType{
	{Name: "monostate", Type: int64(0)},
	{Name: "name", Type: eos.Name("")},
	{Name: "string", Type: ""},
	{Name: "asset", Type: eos.Asset{}}, //(*eos.Asset)(nil)}, // Syntax for pointer to a type, could be any struct
	{Name: "time_point", Type: eos.TimePoint(0)},
	{Name: "int64", Type: int64(0)},
	{Name: "checksum256", Type: eos.Checksum256([]byte("0"))},
})

// GetVariants returns the definition of types compatible with FlexValue
func GetVariants() *eos.VariantDefinition {
	return FlexValueVariant
}

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
		return fmt.Sprint(v)
	case eos.Asset:
		return v.String()
	case string:
		return v
	case eos.TimePoint:
		return v.String()
	case eos.Checksum256:
		return v.String()
	default:
		return fmt.Sprintf("received an unexpected type %T for variant %T", v, fv)
	}
}

// TimePoint returns a eos.TimePoint value of found content
func (fv *FlexValue) TimePoint() (eos.TimePoint, error) {
	switch v := fv.Impl.(type) {
	case eos.TimePoint:
		return v, nil
	default:
		return 0, &InvalidTypeError{
			Label:        fmt.Sprintf("received an unexpected type %T for variant %T", v, fv),
			ExpectedType: "eos.TimePoint",
			FlexValue:    fv,
		}
	}
}

// Asset returns a string value of found content or it panics
func (fv *FlexValue) Asset() (eos.Asset, error) {
	switch v := fv.Impl.(type) {
	case eos.Asset:
		return v, nil
	default:
		return eos.Asset{}, &InvalidTypeError{
			Label:        fmt.Sprintf("received an unexpected type %T for variant %T", v, fv),
			ExpectedType: "eos.Asset",
			FlexValue:    fv,
		}
	}
}

// Name returns a string value of found content or it panics
func (fv *FlexValue) Name() (eos.Name, error) {
	switch v := fv.Impl.(type) {
	case eos.Name:
		return v, nil
	case string:
		return eos.Name(v), nil
	default:
		return eos.Name(""), &InvalidTypeError{
			Label:        fmt.Sprintf("received an unexpected type %T for variant %T", v, fv),
			ExpectedType: "eos.Name",
			FlexValue:    fv,
		}
	}
}

// Int64 returns a string value of found content or it panics
func (fv *FlexValue) Int64() (int64, error) {
	switch v := fv.Impl.(type) {
	case int64:
		return v, nil
	default:
		return -1000000, &InvalidTypeError{
			Label:        fmt.Sprintf("received an unexpected type %T for variant %T", v, fv),
			ExpectedType: "int64",
			FlexValue:    fv,
		}
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
	return fv.BaseVariant.MarshalJSON(FlexValueVariant)
}

// UnmarshalJSON translates flexValueVariant
func (fv *FlexValue) UnmarshalJSON(data []byte) error {
	return fv.BaseVariant.UnmarshalJSON(data, FlexValueVariant)
}

// UnmarshalBinary ...
func (fv *FlexValue) UnmarshalBinary(decoder *eos.Decoder) error {
	return fv.BaseVariant.UnmarshalBinaryVariant(decoder, FlexValueVariant)
}
