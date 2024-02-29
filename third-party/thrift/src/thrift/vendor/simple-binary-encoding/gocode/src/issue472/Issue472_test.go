package issue472

import (
	"bytes"
	"testing"
)

// Optional values should allow NullValue in RangeCheck
func TestOptionalRangeCheck(t *testing.T) {
	var in Issue472
	Issue472Init(&in)
	if err := in.RangeCheck(in.SbeSchemaVersion(), in.SbeSchemaVersion()); err != nil {
		t.Log("Issue472 RangeCheck() did not accept a NullValue for an optional field", err)
		t.Fail()
	}
}

// Optional values should be set to Null Value by the Init() method
func TestOptionalInit(t *testing.T) {
	var in Issue472
	Issue472Init(&in)
	if in.Optional != in.OptionalNullValue() {
		t.Log("Issue472Init() failed to initialize optional field to Null Value")
		t.Fail()
	}
}

func TestEncodeDecode(t *testing.T) {

	m := NewSbeGoMarshaller()
	var in Issue472
	Issue472Init(&in)
	var buf = new(bytes.Buffer)

	// in contains a single optional field which is not initialized
	if err := in.Encode(m, buf, true); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	// Create a new Issue472 and decode into it
	out := *new(Issue472)

	if err := out.Decode(m, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}
	t.Log("Issue472 decodes as: ", out)

	// Sanity checks
	if buf.Len() != 0 {
		t.Log("buffer not drained")
		t.Fail()
	}
	if in != out {
		t.Logf("in != out\n%v\n%v", in, out)
		t.Fail()
	}
}
