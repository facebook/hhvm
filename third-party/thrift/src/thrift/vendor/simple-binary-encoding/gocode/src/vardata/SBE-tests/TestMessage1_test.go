package sbe_tests

import (
	"bytes"
	"testing"
)

func TestEncodeDecodeTestMessage1(t *testing.T) {
	m := NewSbeGoMarshaller()
	var in TestMessage1

	in.EncryptedNewPassword = []byte("abcdefghijklmnopqrst")

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

	t.Logf("in:  len=%d, cap=%d, in=%v\n", len(in.EncryptedNewPassword), cap(in.EncryptedNewPassword), in.EncryptedNewPassword)
	t.Logf("out: len=%d, cap=%d, out=%v\n", len(out.EncryptedNewPassword), cap(out.EncryptedNewPassword), out.EncryptedNewPassword)

	if len(in.EncryptedNewPassword) != len(out.EncryptedNewPassword) {
		t.Logf("len(in.EncryptedNewPassword)(%d) != len(out.EncryptedNewPassword)(%d):\n", len(in.EncryptedNewPassword), len(out.EncryptedNewPassword))
		t.Fail()
	}
	for i := 0; i < len(in.EncryptedNewPassword); i++ {
		if in.EncryptedNewPassword[i] != out.EncryptedNewPassword[i] {
			t.Logf("in.EncryptedNewPassword[%d] != out.EncryptedNewPassword)[%d]:\n", len(in.EncryptedNewPassword), len(out.EncryptedNewPassword))
			t.Fail()
		}
	}

	return

}
