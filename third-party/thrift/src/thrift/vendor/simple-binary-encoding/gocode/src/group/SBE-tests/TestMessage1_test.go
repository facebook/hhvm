package sbe_tests

import (
	"bytes"
	"testing"
)

func TestEncodeDecodeTestMessage1(t *testing.T) {
	m := NewSbeGoMarshaller()

	var s [20]byte
	copy(s[:], "abcdefghijklmnopqrst")

	group := TestMessage1Entries{s, 7}
	var in TestMessage1

	in.Tag1 = 44
	in.Entries = append(in.Entries, group)

	var buf = new(bytes.Buffer)
	if err := in.Encode(m, buf, true); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	var out TestMessage1 = *new(TestMessage1)
	if err := out.Decode(m, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}

	t.Logf("len=%d, cap=%d, in=%v\n", len(in.Entries), cap(in.Entries), in.Entries)

	t.Logf("len=%d, cap=%d, out=%v\n", len(out.Entries), cap(out.Entries), out.Entries)

	if in.Tag1 != out.Tag1 {
		t.Log("in != out:\n", in, out)
		t.Fail()
	}
	if len(in.Entries) != len(out.Entries) {
		t.Logf("len(in.Entries)(%d) != len(out.Entries)(%d):\n", len(in.Entries), len(out.Entries))
		t.Fail()
	}
	for i := 0; i < len(in.Entries); i++ {
		if in.Entries[i] != out.Entries[i] {
			t.Logf("in.Entries[%d] != out.Entries[%d]:\n", in.Entries[i], out.Entries[i])
			t.Fail()
		}
	}

	return

}
