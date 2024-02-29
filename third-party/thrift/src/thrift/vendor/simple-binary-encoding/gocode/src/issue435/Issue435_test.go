package issue435

import (
	"bytes"
	"testing"
)

func TestEncodeDecode(t *testing.T) {
	m := NewSbeGoMarshaller()
	in := Issue435{ExampleRef{EnumRef.One}}
	var s SetRef
	s[SetRefChoice.Two] = true

	// Non-standard header so we use the generated one
	hdr := MessageHeader{in.SbeBlockLength(), in.SbeTemplateId(), in.SbeSchemaId(), in.SbeSchemaVersion(), s}

	var buf = new(bytes.Buffer)
	if err := hdr.Encode(m, buf); err != nil {
		t.Log("MessageHeader Encoding Error", err)
		t.Fail()
	}
	t.Log(hdr, " -> ", buf.Bytes())
	t.Log("Cap() = ", buf.Cap(), "Len() = ", buf.Len())

	if err := in.Encode(m, buf, true); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}
	t.Log(in, " -> ", buf.Bytes())
	t.Log("Cap() = ", buf.Cap(), "Len() = ", buf.Len())

	// Create a new empty MessageHeader and Issue435
	hdr = *new(MessageHeader)
	out := *new(Issue435)

	if err := hdr.Decode(m, buf, out.SbeSchemaVersion()); err != nil {
		t.Log("MessageHeader Decoding Error", err)
		t.Fail()
	}
	t.Log("MessageHeader Decodes as: ", m)
	t.Log("Cap() = ", buf.Cap(), "Len() = ", buf.Len())

	if err := out.Decode(m, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}
	t.Log("Issue435 decodes as: ", out)
	t.Log("Cap() = ", buf.Cap(), "Len() = ", buf.Len())

	// Sanity checks
	if buf.Len() != 0 {
		t.Log("buffer not drained")
		t.Fail()
	}
	if in != out {
		t.Logf("in != out\n%v\n%v", in, out)
		t.Fail()
	}
	if hdr.S != s {
		t.Logf("hdr.S != s (%v != %v)", hdr.S, s)
		t.Fail()
	}
}
