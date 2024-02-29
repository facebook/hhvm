package issue560

import (
	"bytes"
	"testing"
)

func TestInit(t *testing.T) {
	issue560 := new(Issue560)
	Issue560Init(issue560)
	if issue560.DiscountedModel != Model.C {
		t.Log("Issue 560: Init() failed")
		t.Fail()
	}
}

func TestEncodeDecode(t *testing.T) {

	m := NewSbeGoMarshaller()
	var in Issue560
	Issue560Init(&in)
	var buf = new(bytes.Buffer)

	// in contains a single optional field which is not initialized
	if err := in.Encode(m, buf, true); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	// Create a new Issue560 and decode into it
	out := *new(Issue560)

	if err := out.Decode(m, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}
	t.Log("Issue560 decodes as: ", out)

	// Sanity checks
	if buf.Len() != 0 {
		t.Logf("buffer not drained")
		t.Fail()
	}
	if in != out {
		t.Logf("in != out\n%v\n%v", in, out)
		t.Fail()
	}
}
