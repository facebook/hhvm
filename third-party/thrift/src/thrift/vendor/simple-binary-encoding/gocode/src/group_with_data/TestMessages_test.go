package group_with_data

import (
	"bytes"
	_ "fmt"
	"testing"
)

func TestEncodeDecodeTestMessage1(t *testing.T) {
	m := NewSbeGoMarshaller()
	var tm1e TestMessage1Entries
	copy(tm1e.TagGroup1[:], "123456789")
	tm1e.TagGroup2 = 123456789
	tm1e.VarDataField = []byte("abcdef")

	var in TestMessage1
	in.Tag1 = 1234
	in.Entries = append(in.Entries, tm1e)

	var buf = new(bytes.Buffer)
	if err := in.Encode(m, buf, true); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	var out = *new(TestMessage1)
	if err := out.Decode(m, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}

	// fmt.Printf("%+v\n", in)
	// fmt.Printf("%+v\n", out)

	if in.Tag1 != out.Tag1 {
		t.Log("in.Tag1 != out.Tag1")
		t.Fail()
	}
	if in.Entries[0].TagGroup1 != out.Entries[0].TagGroup1 {
		t.Log("in.Entries[0].TagGroup1 != out.Entries[0].TagGroup1")
		t.Fail()
	}
	if in.Entries[0].TagGroup2 != out.Entries[0].TagGroup2 {
		t.Log("in.Entries[0].TagGroup2 != out.Entries[0].TagGroup2")
		t.Fail()
	}
	if !bytes.Equal(in.Entries[0].VarDataField, out.Entries[0].VarDataField) {
		t.Log("in.Entries[0].VarDataField != out.Entries[0].VarDataField", in.Entries[0].VarDataField, out.Entries[0].VarDataField)
		t.Fail()
	}
	return
}

func TestEncodeDecodeTestMessage2(t *testing.T) {
	m := NewSbeGoMarshaller()

	var tm2e TestMessage2Entries
	copy(tm2e.TagGroup1[:], "123456789")
	tm2e.TagGroup2 = 123456789
	tm2e.VarDataField1 = []byte("abcdef")
	tm2e.VarDataField2 = []byte("ghij")

	var in TestMessage2
	in.Tag1 = 1234
	in.Entries = append(in.Entries, tm2e)
	in.Entries = append(in.Entries, tm2e)

	var buf = new(bytes.Buffer)
	if err := in.Encode(m, buf, true); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	var out = *new(TestMessage2)
	if err := out.Decode(m, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}

	// fmt.Printf("%+v\n", in)
	// fmt.Printf("%+v\n", out)

	if in.Tag1 != out.Tag1 {
		t.Log("in.Tag1 != out.Tag1")
		t.Fail()
	}

	for i := 0; i < len(in.Entries); i++ {
		if in.Entries[0].TagGroup1 != out.Entries[0].TagGroup1 {
			t.Log("in.Entries[0].TagGroup1 != out.Entries[0].TagGroup1")
			t.Fail()
		}
		if in.Entries[0].TagGroup2 != out.Entries[0].TagGroup2 {
			t.Log("in.Entries[0].TagGroup2 != out.Entries[0].TagGroup2")
			t.Fail()
		}
		if !bytes.Equal(in.Entries[i].VarDataField1, out.Entries[i].VarDataField1) {
			t.Logf("in.Entries[%d].VarDataField (%v)!= out.Entries[%d].VarDataField (%v)", i, i, in.Entries[i].VarDataField1, out.Entries[i].VarDataField1)
			t.Fail()
		}
		if !bytes.Equal(in.Entries[i].VarDataField2, out.Entries[i].VarDataField2) {
			t.Logf("in.Entries[%d].VarDataField (%v) != out.Entries[%d].VarDataField (%v)", i, i, in.Entries[i].VarDataField2, out.Entries[i].VarDataField2)
			t.Fail()
		}
	}
	return
}

func TestEncodeDecodeTestMessage3(t *testing.T) {
	m := NewSbeGoMarshaller()

	var tm3en TestMessage3EntriesNestedEntries
	tm3en.TagGroup2 = 99887766
	tm3en.VarDataFieldNested = []byte("nested")

	var tm3e TestMessage3Entries
	copy(tm3e.TagGroup1[:], "123456789")
	tm3e.NestedEntries = append(tm3e.NestedEntries, tm3en)
	tm3e.VarDataField = []byte("middle")

	var in TestMessage3
	in.Tag1 = 1234
	in.Entries = append(in.Entries, tm3e)
	in.Entries = append(in.Entries, tm3e)

	var buf = new(bytes.Buffer)
	if err := in.Encode(m, buf, true); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	var out = *new(TestMessage3)
	if err := out.Decode(m, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}

	// fmt.Printf("%+v\n", in)
	// fmt.Printf("%+v\n", out)

	if in.Tag1 != out.Tag1 {
		t.Log("in.Tag1 != out.Tag1")
		t.Fail()
	}

	for i := 0; i < len(in.Entries); i++ {
		if in.Entries[0].TagGroup1 != out.Entries[0].TagGroup1 {
			t.Log("in.Entries[0].TagGroup1 != out.Entries[0].TagGroup1")
			t.Fail()
		}
		if !bytes.Equal(in.Entries[i].VarDataField, out.Entries[i].VarDataField) {
			t.Logf("in.Entries[%d].VarDataField (%v) != out.Entries[%d].VarDataField (%v)", i, i, in.Entries[i].VarDataField, out.Entries[i].VarDataField)
			t.Fail()
		}

		if in.Entries[0].NestedEntries[0].TagGroup2 != out.Entries[0].NestedEntries[0].TagGroup2 {
			t.Log("if in.Entries[0].NestedEntries[0].TagGroup2 != out.Entries[0].NestedEntries[0].TagGroup2")
			t.Fail()
		}

		if !bytes.Equal(in.Entries[0].NestedEntries[0].VarDataFieldNested, out.Entries[0].NestedEntries[0].VarDataFieldNested) {
			t.Log("in.Entries[0].NestedEntries[0].VarDataFieldNested != out.Entries[0].NestedEntries[0].VarDataFieldNested")
			t.Fail()
		}

	}
	return
}

func TestEncodeDecodeTestMessage4(t *testing.T) {
	m := NewSbeGoMarshaller()

	var tm4e TestMessage4Entries
	tm4e.VarDataField1 = []byte("abcdef")
	tm4e.VarDataField2 = []byte("ghij")

	var in TestMessage4
	in.Tag1 = 9876
	in.Entries = append(in.Entries, tm4e)
	in.Entries = append(in.Entries, tm4e)
	in.Entries = append(in.Entries, tm4e)

	var buf = new(bytes.Buffer)
	if err := in.Encode(m, buf, true); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	var out = *new(TestMessage4)
	if err := out.Decode(m, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}

	// fmt.Printf("%+v\n", in)
	// fmt.Printf("%+v\n", out)

	if in.Tag1 != out.Tag1 {
		t.Logf("in.Tag1 != out.Tag1")
		t.Fail()
	}

	for i := 0; i < len(in.Entries); i++ {
		if !bytes.Equal(in.Entries[i].VarDataField1, out.Entries[i].VarDataField1) {
			t.Logf("in.Entries[%d].VarDataField (%v)!= out.Entries[%d].VarDataField (%v)", i, i, in.Entries[i].VarDataField1, out.Entries[i].VarDataField1)
			t.Fail()
		}
		if !bytes.Equal(in.Entries[i].VarDataField2, out.Entries[i].VarDataField2) {
			t.Logf("in.Entries[%d].VarDataField (%v) != out.Entries[%d].VarDataField (%v)", i, i, in.Entries[i].VarDataField2, out.Entries[i].VarDataField2)
			t.Fail()
		}
	}
	return
}

func TestEncodeDecodeTestPreAlloc(t *testing.T) {
	m := NewSbeGoMarshaller()

	in := TestMessage4{
		Tag1: 9876,
		Entries: []TestMessage4Entries{
			{
				VarDataField1: []byte("abcdef"),
				VarDataField2: []byte("ghij"),
			},
			{
				VarDataField1: []byte("abc"),
				VarDataField2: []byte("gh"),
			},
		},
	}

	var buf = new(bytes.Buffer)
	if err := in.Encode(m, buf, true); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	var out = *new(TestMessage4)
	if err := out.Decode(m, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}

	if in.Tag1 != out.Tag1 {
		t.Logf("in.Tag1 != out.Tag1")
		t.Fail()
	}

	for i := 0; i < len(in.Entries); i++ {
		if !bytes.Equal(in.Entries[i].VarDataField1, out.Entries[i].VarDataField1) {
			t.Logf("in.Entries[%d].VarDataField (%v)!= out.Entries[%d].VarDataField (%v)", i, i, in.Entries[i].VarDataField1, out.Entries[i].VarDataField1)
			t.Fail()
		}
		if !bytes.Equal(in.Entries[i].VarDataField2, out.Entries[i].VarDataField2) {
			t.Logf("in.Entries[%d].VarDataField (%v) != out.Entries[%d].VarDataField (%v)", i, i, in.Entries[i].VarDataField2, out.Entries[i].VarDataField2)
			t.Fail()
		}
	}

	// new messages with newer group elements, shorter vardata
	in = TestMessage4{
		Tag1: 9876,
		Entries: []TestMessage4Entries{
			{
				VarDataField1: []byte("abc"),
				VarDataField2: []byte("ghijk"),
			},
		},
	}

	buf.Reset()
	if err := in.Encode(m, buf, true); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	if err := out.Decode(m, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}

	if in.Tag1 != out.Tag1 {
		t.Logf("in.Tag1 != out.Tag1")
		t.Fail()
	}

	for i := 0; i < len(in.Entries); i++ {
		if !bytes.Equal(in.Entries[i].VarDataField1, out.Entries[i].VarDataField1) {
			t.Logf("in.Entries[%d].VarDataField (%v)!= out.Entries[%d].VarDataField (%v)", i, i, in.Entries[i].VarDataField1, out.Entries[i].VarDataField1)
			t.Fail()
		}
		if !bytes.Equal(in.Entries[i].VarDataField2, out.Entries[i].VarDataField2) {
			t.Logf("in.Entries[%d].VarDataField (%v) != out.Entries[%d].VarDataField (%v)", i, i, in.Entries[i].VarDataField2, out.Entries[i].VarDataField2)
			t.Fail()
		}
	}

	return
}
