/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

package format

import (
	"math"
	"strconv"
)

type Numeric interface {
	Int64() int64
	Int32() int32
	Int16() int16
	Byte() byte
	Int() int
	Float64() float64
	Float32() float32
	String() string
	isNull() bool
}

type numeric struct {
	iValue int64
	dValue float64
	sValue string
	isNil  bool
}

var (
	INFINITY          Numeric = &numeric{iValue: 0, dValue: math.Inf(1), sValue: "Infinity", isNil: false}
	NEGATIVE_INFINITY Numeric = &numeric{iValue: 0, dValue: math.Inf(-1), sValue: "-Infinity", isNil: false}
	NAN               Numeric = &numeric{iValue: 0, dValue: math.NaN(), sValue: "NaN", isNil: false}
	ZERO              Numeric = &numeric{iValue: 0, dValue: 0, sValue: "0", isNil: false}
	NUMERIC_NULL      Numeric = &numeric{iValue: 0, dValue: 0, sValue: "0", isNil: true}
)

func NewNumericFromDouble(dValue float64) Numeric {
	if math.IsInf(dValue, 1) {
		return INFINITY
	}
	if math.IsInf(dValue, -1) {
		return NEGATIVE_INFINITY
	}
	if math.IsNaN(dValue) {
		return NAN
	}
	iValue := int64(dValue)
	sValue := strconv.FormatFloat(dValue, 'g', 10, 64)
	isNil := false
	return &numeric{iValue: iValue, dValue: dValue, sValue: sValue, isNil: isNil}
}

func NewNumericFromFloat(dValue float32) Numeric {
	if math.IsInf(float64(dValue), 1) {
		return INFINITY
	}
	if math.IsInf(float64(dValue), -1) {
		return NEGATIVE_INFINITY
	}
	if math.IsNaN(float64(dValue)) {
		return NAN
	}
	iValue := int32(dValue)
	sValue := strconv.FormatFloat(float64(dValue), 'g', 10, 32)
	isNil := false
	return &numeric{iValue: int64(iValue), dValue: float64(dValue), sValue: sValue, isNil: isNil}
}

func NewNumericFromI64(iValue int64) Numeric {
	dValue := float64(iValue)
	sValue := strconv.FormatInt(iValue, 10)
	isNil := false
	return &numeric{iValue: iValue, dValue: dValue, sValue: sValue, isNil: isNil}
}

func NewNumericFromI32(iValue int32) Numeric {
	dValue := float64(iValue)
	sValue := string(iValue)
	isNil := false
	return &numeric{iValue: int64(iValue), dValue: dValue, sValue: sValue, isNil: isNil}
}

func NewNumericFromString(sValue string) Numeric {
	if sValue == INFINITY.String() {
		return INFINITY
	}
	if sValue == NEGATIVE_INFINITY.String() {
		return NEGATIVE_INFINITY
	}
	if sValue == NAN.String() {
		return NAN
	}
	iValue, _ := strconv.ParseInt(sValue, 10, 64)
	dValue, _ := strconv.ParseFloat(sValue, 64)
	isNil := len(sValue) == 0
	return &numeric{iValue: iValue, dValue: dValue, sValue: sValue, isNil: isNil}
}

func NewNumericFromJSONString(sValue string, isNull bool) Numeric {
	if isNull {
		return NewNullNumeric()
	}
	if sValue == JSON_INFINITY {
		return INFINITY
	}
	if sValue == JSON_NEGATIVE_INFINITY {
		return NEGATIVE_INFINITY
	}
	if sValue == JSON_NAN {
		return NAN
	}
	iValue, _ := strconv.ParseInt(sValue, 10, 64)
	dValue, _ := strconv.ParseFloat(sValue, 64)
	return &numeric{iValue: iValue, dValue: dValue, sValue: sValue, isNil: isNull}
}

func NewNullNumeric() Numeric {
	return &numeric{iValue: 0, dValue: 0.0, sValue: "", isNil: true}
}

func (p *numeric) Int64() int64 {
	return p.iValue
}

func (p *numeric) Int32() int32 {
	return int32(p.iValue)
}

func (p *numeric) Int16() int16 {
	return int16(p.iValue)
}

func (p *numeric) Byte() byte {
	return byte(p.iValue)
}

func (p *numeric) Int() int {
	return int(p.iValue)
}

func (p *numeric) Float64() float64 {
	return p.dValue
}

func (p *numeric) Float32() float32 {
	return float32(p.dValue)
}

func (p *numeric) String() string {
	return p.sValue
}

func (p *numeric) isNull() bool {
	return p.isNil
}
