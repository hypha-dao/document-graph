package docgraph

// ContentGroup ...
type ContentGroup []ContentItem

// GetContent returns a FlexValue of the content with the matching label
// or an instance of ContentNotFoundError
func (cg *ContentGroup) GetContent(label string) (*FlexValue, error) {
	for _, content := range *cg {
		if content.Label == label {
			return content.Value, nil
		}
	}

	return nil, &ContentNotFoundError{
		Label: label,
	}
}
