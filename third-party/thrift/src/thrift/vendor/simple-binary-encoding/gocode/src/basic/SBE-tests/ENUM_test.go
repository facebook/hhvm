package sbe_tests

import (
	"bytes"
	"testing"
)

func TestEncodeDecodeEnum(t *testing.T) {

	m := NewSbeGoMarshaller()

	// var e ENUMEnum = Value1;
	var in ENUMEnum = ENUM.Value10
	var buf = new(bytes.Buffer)
	if err := in.Encode(m, buf); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	var out ENUMEnum = *new(ENUMEnum)
	if err := out.Decode(m, buf, 0); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}

	if in != out {
		t.Log("in != out:\n", in, out)
		t.Fail()
	}

	return

	// xmain()
}

func TestEnumRange(t *testing.T) {

	m := NewSbeGoMarshaller()
	var in ENUMEnum = 9

	// newer version should be ok
	if err := in.RangeCheck(1, 0); err != nil {
		t.Log("RangeCheck failed and should have passed", err)
		t.Fail()
	}

	// same version should fail
	if err := in.RangeCheck(0, 0); err == nil {
		t.Log("RangeCheck passed and should have failed", err)
		t.Fail()
	}

	// older version should fail
	if err := in.RangeCheck(0, 1); err == nil {
		t.Log("RangeCheck passed and should have failed", err)
		t.Fail()
	}

	// Now let's encode and decode that (no rangecheck performed)
	// valie in the message
	var buf = new(bytes.Buffer)
	if err := in.Encode(m, buf); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	var out ENUMEnum = *new(ENUMEnum)
	if err := out.Decode(m, buf, 0); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}

	// Values should the be the same
	if in != out {
		t.Log("in != out:\n", in, out)
		t.Fail()
	}

	// But a rangeheck might fail
	// newer version should be ok
	if err := out.RangeCheck(1, 0); err != nil {
		t.Log("RangeCheck failed and should have passed", err)
		t.Fail()
	}

	// same version should fail
	if err := out.RangeCheck(0, 0); err == nil {
		t.Log("RangeCheck passed and should have failed", err)
		t.Fail()
	}

	// older version should fail
	if err := out.RangeCheck(0, 1); err == nil {
		t.Log("RangeCheck passed and should have failed", err)
		t.Fail()
	}
	return
}
