package group_with_data_extension

import (
	"bytes"
	"fmt"
	"group_with_data"
	"math"
	"testing"
)

// Note, this is a copy of group-with-data that we extended to test
// message extensions within a nested group and vardata
func makeTestMessage3Extension() TestMessage3 {
	var t TestMessage3

	var tm3en TestMessage3EntriesNestedEntries
	tm3en.TagGroup2 = 99887766
	tm3en.InnerExtension = 11112222 // Only in extension
	tm3en.VarDataFieldNested = []byte("nested")

	var tm3e TestMessage3Entries
	copy(tm3e.TagGroup1[:], "123456789")
	tm3e.NestedEntries = append(tm3e.NestedEntries, tm3en)
	tm3e.VarDataField = []byte("middle")

	t.Tag1 = 1234
	t.Entries = append(t.Entries, tm3e)
	t.Entries = append(t.Entries, tm3e)

	return t
}

func makeTestMessage3Original() group_with_data.TestMessage3 {
	var t group_with_data.TestMessage3

	var tm3en group_with_data.TestMessage3EntriesNestedEntries
	tm3en.TagGroup2 = 99887766
	tm3en.VarDataFieldNested = []byte("nested")

	var tm3e group_with_data.TestMessage3Entries
	copy(tm3e.TagGroup1[:], "123456789")
	tm3e.NestedEntries = append(tm3e.NestedEntries, tm3en)
	tm3e.VarDataField = []byte("middle")

	t.Tag1 = 1234
	t.Entries = append(t.Entries, tm3e)
	t.Entries = append(t.Entries, tm3e)

	return t
}

// Basic test of new message
func TestEncodeDecodeNewtoNew(t *testing.T) {
	m := NewSbeGoMarshaller()

	var buf = new(bytes.Buffer)
	in := makeTestMessage3Extension()

	if err := in.Encode(m, buf, true); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	var out TestMessage3 = *new(TestMessage3)
	if err := out.Decode(m, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}

	if in.Tag1 != out.Tag1 {
		t.Logf("in.Tag1 != out.Tag1")
		t.Fail()
	}

	for i := 0; i < len(in.Entries); i++ {
		if in.Entries[0].TagGroup1 != out.Entries[0].TagGroup1 {
			t.Logf("in.Entries[0].TagGroup1 != out.Entries[0].TagGroup1")
			fmt.Printf("%+v\n%+v\n", in, out)
			t.Fail()
		}
		if !bytes.Equal(in.Entries[i].VarDataField, out.Entries[i].VarDataField) {
			t.Logf("in.Entries[%d].VarDataField (%v) != out.Entries[%d].VarDataField (%v)", i, i, in.Entries[i].VarDataField, out.Entries[i].VarDataField)
			fmt.Printf("%+v\n%+v\n", in, out)
			t.Fail()
		}

		if in.Entries[0].NestedEntries[0].TagGroup2 != out.Entries[0].NestedEntries[0].TagGroup2 {
			t.Logf("in.Entries[0].NestedEntries[0].TagGroup2 != out.Entries[0].NestedEntries[0].TagGroup2")
			fmt.Printf("%+v\n%+v\n", in, out)
			t.Fail()
		}
		if in.Entries[0].NestedEntries[0].InnerExtension != out.Entries[0].NestedEntries[0].InnerExtension {
			t.Logf("n.Entries[0].NestedEntries[0].InnerExtension != out.Entries[0].NestedEntries[0].InnerExtension")
			fmt.Printf("%+v\n%+v\n", in, out)
			t.Fail()
		}

		if !bytes.Equal(in.Entries[0].NestedEntries[0].VarDataFieldNested, out.Entries[0].NestedEntries[0].VarDataFieldNested) {
			t.Logf("in.Entries[0].NestedEntries[0].VarDataFieldNested != out.Entries[0].NestedEntries[0].VarDataFieldNested")
			fmt.Printf("%+v\n%+v\n", in, out)
			t.Fail()
		}

	}
	return
}

// Test of New to Old
func TestEncodeDecodeNewToOld(t *testing.T) {
	min := NewSbeGoMarshaller()
	mout := group_with_data.NewSbeGoMarshaller()

	var buf = new(bytes.Buffer)
	in := makeTestMessage3Extension()

	if err := in.Encode(min, buf, true); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	var out group_with_data.TestMessage3 = *new(group_with_data.TestMessage3)
	if err := out.Decode(mout, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}

	if in.Tag1 != out.Tag1 {
		t.Logf("in.Tag1 != out.Tag1")
		t.Fail()
	}

	for i := 0; i < len(in.Entries); i++ {
		if in.Entries[0].TagGroup1 != out.Entries[0].TagGroup1 {
			t.Logf("in.Entries[0].TagGroup1 != out.Entries[0].TagGroup1")
			// fmt.Printf("%+v\n%+v\n", in, out)
			t.Fail()
		}
		if !bytes.Equal(in.Entries[i].VarDataField, out.Entries[i].VarDataField) {
			t.Logf("in.Entries[%d].VarDataField (%v) != out.Entries[%d].VarDataField (%v)", i, i, in.Entries[i].VarDataField, out.Entries[i].VarDataField)
			// fmt.Printf("%+v\n%+v\n", in, out)
			t.Fail()
		}

		if in.Entries[0].NestedEntries[0].TagGroup2 != out.Entries[0].NestedEntries[0].TagGroup2 {
			t.Logf("in.Entries[0].NestedEntries[0].TagGroup2 != out.Entries[0].NestedEntries[0].TagGroup2")
			// fmt.Printf("%+v\n%+v\n", in, out)
			t.Fail()
		}

		if !bytes.Equal(in.Entries[0].NestedEntries[0].VarDataFieldNested, out.Entries[0].NestedEntries[0].VarDataFieldNested) {
			t.Logf("in.Entries[0].NestedEntries[0].VarDataFieldNested != out.Entries[0].NestedEntries[0].VarDataFieldNested")
			// fmt.Printf("%+v\n%+v\n", in, out)
			t.Fail()
		}

	}
	// fmt.Printf("%+v\n%+v\n", in, out)
	return
}

// Test of Old to New
func TestEncodeDecodeOldToNew(t *testing.T) {
	min := group_with_data.NewSbeGoMarshaller()
	mout := NewSbeGoMarshaller()

	var buf = new(bytes.Buffer)
	in := makeTestMessage3Original()

	if err := in.Encode(min, buf, true); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	var out TestMessage3 = *new(TestMessage3)
	if err := out.Decode(mout, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}

	if in.Tag1 != out.Tag1 {
		t.Logf("in.Tag1 != out.Tag1")
		t.Fail()
	}

	for i := 0; i < len(in.Entries); i++ {
		if in.Entries[0].TagGroup1 != out.Entries[0].TagGroup1 {
			t.Logf("in.Entries[0].TagGroup1 != out.Entries[0].TagGroup1")
			// fmt.Printf("%+v\n%+v\n", in, out)
			t.Fail()
		}
		if !bytes.Equal(in.Entries[i].VarDataField, out.Entries[i].VarDataField) {
			t.Logf("in.Entries[%d].VarDataField (%v) != out.Entries[%d].VarDataField (%v)", i, i, in.Entries[i].VarDataField, out.Entries[i].VarDataField)
			// fmt.Printf("%+v\n%+v\n", in, out)
			t.Fail()
		}

		if in.Entries[0].NestedEntries[0].TagGroup2 != out.Entries[0].NestedEntries[0].TagGroup2 {
			t.Logf("in.Entries[0].NestedEntries[0].TagGroup2 != out.Entries[0].NestedEntries[0].TagGroup2")
			// fmt.Printf("%+v\n%+v\n", in, out)
			t.Fail()
		}

		if math.MinInt64 != out.Entries[0].NestedEntries[0].InnerExtension {
			t.Logf("0 != out.Entries[0].NestedEntries[0].InnerExtension (%v)", out.Entries[0].NestedEntries[0].InnerExtension)
			// fmt.Printf("%+v\n%+v\n\n", in, out)
			t.Fail()
		}
		if !bytes.Equal(in.Entries[0].NestedEntries[0].VarDataFieldNested, out.Entries[0].NestedEntries[0].VarDataFieldNested) {
			t.Logf("in.Entries[0].NestedEntries[0].VarDataFieldNested != out.Entries[0].NestedEntries[0].VarDataFieldNested")
			// fmt.Printf("%+v\n%+v\n", in, out)
			t.Fail()
		}

	}
	// fmt.Printf("%+v\n%+v\n", in, out)
	return
}
