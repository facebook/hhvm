/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package gotest

import (
	"reflect"
	"sort"
	"testing"
	"thrift/test/go/if/thrifttest"
)

func TestStructMapKey(t *testing.T) {
	k1a := thrifttest.MapKey{Num: 1, Strval: "a"}
	k1b := thrifttest.MapKey{Num: 1, Strval: "a"}
	k2 := thrifttest.MapKey{Num: 2, Strval: "b"}

	maps := thrifttest.Maps{Struct2str: map[thrifttest.MapKey]string{}}
	maps.Struct2str[k1a] = "x"
	maps.Struct2str[k1b] = "y"
	maps.Struct2str[k2] = "z"

	if maps.Struct2str[k1a] != "y" {
		t.Fatalf("unexpected map value (should have been overwritten to 'y')")
	}

	if maps.Struct2str[k2] != "z" {
		t.Fatalf("unexpected map value (should have been 'z')")
	}

	if len(maps.Struct2str) != 2 {
		t.Fatalf("unexpected map length (should have been 2)")
	}
}

func TestEnumValues(t *testing.T) {
	generatedValues := thrifttest.NumberzValues
	sort.Slice(generatedValues, func(i, j int) bool { return generatedValues[i] < generatedValues[j] })

	values := []thrifttest.Numberz{}
	for k := range thrifttest.NumberzToName {
		values = append(values, k)
	}
	sort.Slice(values, func(i, j int) bool { return values[i] < values[j] })

	if !reflect.DeepEqual(generatedValues, values) {
		t.Fatalf("values slices are not equal (should be equal)")
	}
}

func TestEnumNames(t *testing.T) {
	generatedNames := thrifttest.NumberzNames
	sort.Strings(generatedNames)

	names := []string{}
	for k := range thrifttest.NumberzToValue {
		names = append(names, k)
	}
	sort.Strings(names)

	if !reflect.DeepEqual(generatedNames, names) {
		t.Fatalf("names slices are not equal (should be equal)")
	}
}

func TestBuilder(t *testing.T) {
	bonk1 := thrifttest.NewBonk()
	bonk1.Message = "exampleMessage"
	bonk1.Type = 3

	bonkBuilder := thrifttest.NewBonkBuilder()
	bonk2 := bonkBuilder.
		Message("exampleMessage").
		Type(3).
		Emit()
	bonk3 := bonkBuilder.
		Message("anotherMessage").
		Emit()

	wn1 := thrifttest.NewWeirdNames()
	wn1.Me = true
	wn1.SetMe = true
	wn1.SetMe_ = true
	wn1.P = true
	wn1.B = true

	wn2 := thrifttest.NewWeirdNamesBuilder().
		Me(true).
		SetMe(true).
		SetMe_(true).
		P(true).
		B(true).
		Emit()

	if !reflect.DeepEqual(wn1, wn2) {
		t.Fatalf("structs are not equal (should be equal)")
	}

	if !reflect.DeepEqual(bonk1, bonk2) {
		t.Fatalf("structs are not equal (should be equal)")
	}
	if bonk2.Type != bonk3.Type {
		t.Fatalf("types are not equal (should be equal)")
	}
	if bonk2.Message == bonk3.Message {
		t.Fatalf("messages are equal (should not be equal)")
	}

	xt1 := thrifttest.NewXtruct4()
	xt1.IntThing = 0

	xt2 := thrifttest.NewXtruct4Builder().
		IntThing(0).
		Emit()

	if !reflect.DeepEqual(xt1, xt2) {
		t.Fatalf("structs are not equal (should be equal)")
	}

	xt3 := thrifttest.NewXtruct4()
	xt3.Xtruct2 = thrifttest.NewXtruct2()
	xt3.Xtruct2.StructThing = thrifttest.NewXtruct()
	xt3.Xtruct2.StructThing.StringThing = "exampleString"

	xt4 := thrifttest.NewXtruct4Builder().
		Xtruct2(thrifttest.NewXtruct2Builder().
			StructThing(thrifttest.NewXtructBuilder().
				StringThing("exampleString").
				Emit(),
			).
			Emit(),
		).
		Emit()

	if !reflect.DeepEqual(xt3, xt4) {
		t.Fatalf("structs are not equal (should be equal)")
	}
}

func TestSetters(t *testing.T) {
	bonk1 := thrifttest.NewBonk()
	bonk1.Message = "exampleMessage"
	bonk1.Type = 3

	bonk2 := thrifttest.NewBonk().
		SetMessage("exampleMessage").
		SetType(3)

	if !reflect.DeepEqual(bonk1, bonk2) {
		t.Fatalf("structs are not equal (should be equal)")
	}

	wn1 := thrifttest.NewWeirdNames()
	wn1.Me = true
	wn1.SetMe = true
	wn1.SetMe_ = true
	wn1.P = true
	wn1.B = true

	wn2 := thrifttest.NewWeirdNames().
		SetMe__(true).
		SetSetMe(true).
		SetSetMe_(true).
		SetP(true).
		SetB(true)

	if !reflect.DeepEqual(wn1, wn2) {
		t.Fatalf("structs are not equal (should be equal)")
	}

	wn2.SetXSetMe(true)
	if reflect.DeepEqual(wn1, wn2) {
		t.Fatalf("structs are equal (should be not equal)")
	}

	xt1 := thrifttest.NewXtruct4()
	xt1.IntThing = 0

	xt2 := thrifttest.NewXtruct4().
		SetIntThing(0)

	if !reflect.DeepEqual(xt1, xt2) {
		t.Fatalf("structs are not equal (should be equal)")
	}

	xt3 := thrifttest.NewXtruct4()
	xt3.Xtruct2 = thrifttest.NewXtruct2()
	xt3.Xtruct2.StructThing.StringThing = "exampleString"

	xt4 := thrifttest.NewXtruct4().
		SetXtruct2(thrifttest.NewXtruct2().
			SetStructThing(thrifttest.NewXtruct().
				SetStringThing("exampleString"),
			),
		)

	if !reflect.DeepEqual(xt3, xt4) {
		t.Fatalf("structs are not equal (should be equal)")
	}
}
