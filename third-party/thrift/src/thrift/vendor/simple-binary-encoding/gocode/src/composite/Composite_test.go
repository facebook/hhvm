package composite

import (
	"bytes"
	"testing"
)

func TestEncodeDecode(t *testing.T) {
	m := NewSbeGoMarshaller()

	var s1, s2 [5]byte
	copy(s1[:], "start")
	copy(s2[:], "  end")
	p1 := Point{s1, 3.14, 1, [2]uint8{66, 77}, BooleanEnum.NullValue, BooleanEnum.T}
	p2 := Point{s2, 0.31, 2, [2]uint8{77, 88}, BooleanEnum.T, BooleanEnum.F}
	in := Composite{p1, p2}

	var cbuf = new(bytes.Buffer)
	if err := in.Encode(m, cbuf, true); err != nil {
		t.Log("Composite Encoding Error", err)
		t.Fail()
	}
	t.Log(in, " -> ", cbuf.Bytes())
	t.Log("Cap() = ", cbuf.Cap(), "Len() = \n", cbuf.Len())

	hdr := SbeGoMessageHeader{in.SbeBlockLength(), in.SbeTemplateId(), in.SbeSchemaId(), in.SbeSchemaVersion()}
	var mbuf = new(bytes.Buffer)
	if err := hdr.Encode(m, mbuf); err != nil {
		t.Log("MessageHeader Encoding Error", err)
		t.Fail()
	}
	t.Log(hdr, " -> ", mbuf.Bytes())
	t.Log("Cap() = ", mbuf.Cap(), "Len() = \n", mbuf.Len())

	// Create a new empty MessageHeader and Composite
	hdr = *new(SbeGoMessageHeader)
	var out Composite = *new(Composite)

	if err := hdr.Decode(m, mbuf); err != nil {
		t.Log("MessageHeader Decoding Error", err)
		t.Fail()
	}
	t.Log("MessageHeader Decodes as: ", m)
	t.Log("Cap() = ", mbuf.Cap(), "Len() = \n", mbuf.Len())

	if err := out.Decode(m, cbuf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Composite Decoding Error", err)
		t.Fail()
	}
	t.Log("Composite decodes as: ", out)
	t.Log("Cap() = ", cbuf.Cap(), "Len() = \n", cbuf.Len())

	if in != out {
		t.Logf("in != out\n%v\n%v", in, out)
		t.Fail()
	}
}
