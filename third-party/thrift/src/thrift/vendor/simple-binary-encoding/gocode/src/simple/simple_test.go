package simple

import (
	"bytes"
	"testing"
)

func TestEncodeDecode(t *testing.T) {
	m := NewSbeGoMarshaller()

	in := Simple0{2863311530, 2863311530, 123456, 7890, 63, -8, -16, -32, -64, 3.14, -3.14e7, [6]byte{'a', 'b', 'c', 'd', 'e', 'f'}, 'A', [2]int32{254, 255}}

	var sbuf = new(bytes.Buffer)
	if err := in.Encode(m, sbuf, true); err != nil {
		t.Log("Simple0 Encoding Error", err)
		t.Fail()
	}
	t.Log(in, " -> ", sbuf.Bytes())
	t.Log("Cap() = ", sbuf.Cap(), "Len() = ", sbuf.Len())

	hdr := MessageHeader{in.SbeBlockLength(), in.SbeTemplateId(), in.SbeSchemaId(), in.SbeSchemaVersion()}
	var mbuf = new(bytes.Buffer)
	if err := hdr.Encode(m, mbuf); err != nil {
		t.Log("MessageHeader Encoding Error", err)
		t.Fail()
	}
	t.Log(hdr, " -> ", mbuf.Bytes())
	t.Log("Cap() = ", mbuf.Cap(), "Len() = ", mbuf.Len())

	// Create a new empty MessageHeader and Simple0
	hdr = *new(MessageHeader)
	var out Simple0 = *new(Simple0)

	// Note generated standard header in use (as blocklength is uint8)
	if err := hdr.Decode(m, mbuf, in.SbeSchemaVersion()); err != nil {
		t.Log("MessageHeader Decoding Error", err)
		t.Fail()
	}
	t.Log("MessageHeader Decodes as: ", m)
	t.Log("Cap() = ", mbuf.Cap(), "Len() = ", mbuf.Len())

	if err := out.Decode(m, sbuf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Simple0 Decoding Error", err)
		t.Fail()
	}
	t.Log("Simple0 decodes as: ", out)
	t.Log("Cap() = ", sbuf.Cap(), "Len() = ", sbuf.Len())

	if in != out {
		t.Logf("in != out\n%v\n%v", in, out)
		t.Fail()
	}

	// SinceVersion and Deprecated checkeds
	if in.U64SinceVersion() != 1 {
		t.Log("in.U64Deprecated() should be 1 and is", in.U64SinceVersion())
		t.Fail()
	}
	if in.U64Deprecated() != 2 {
		t.Log("in.U64Deprecated() should be 2 and is", in.U64Deprecated())
		t.Fail()
	}
}
