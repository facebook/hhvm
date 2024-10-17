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

package types

// FieldSpec is a spec for a struct field.
type FieldSpec struct {
	ID            int16
	WireType      Type
	Name          string
	ReflectIndex  int
	IsOptional    bool
	ValueTypeSpec *TypeSpec
}

// StructSpec is a spec for a stuct.
type StructSpec struct {
	Name           string
	IsUnion        bool
	IsException    bool
	FieldSpecsByID map[int16]*FieldSpec
}

// CodecPrimitiveType is an enum for all primitive types used by codec.
type CodecPrimitiveType int32

const (
	CODEC_PRIMITIVE_TYPE_BOOL CodecPrimitiveType = iota
	CODEC_PRIMITIVE_TYPE_BYTE
	CODEC_PRIMITIVE_TYPE_I16
	CODEC_PRIMITIVE_TYPE_I32
	CODEC_PRIMITIVE_TYPE_I64
	CODEC_PRIMITIVE_TYPE_FLOAT
	CODEC_PRIMITIVE_TYPE_DOUBLE
	CODEC_PRIMITIVE_TYPE_BINARY
	CODEC_PRIMITIVE_TYPE_STRING
	CODEC_PRIMITIVE_TYPE_VOID
)

// CodecPrimitiveSpec is a spec for a primitive type.
type CodecPrimitiveSpec struct {
	PrimitiveType CodecPrimitiveType
}

// CodecEnumSpec is a spec for an enum type.
type CodecEnumSpec struct{}

// CodecSetSpec is a spec for a set type.
type CodecSetSpec struct {
	ElementWireType Type
	ElementTypeSpec *TypeSpec
}

// CodecListSpec is a spec for a list type.
type CodecListSpec struct {
	ElementWireType Type
	ElementTypeSpec *TypeSpec
}

// CodecMapSpec is a spec for a map type.
type CodecMapSpec struct {
	KeyWireType   Type
	ValueWireType Type
	KeyTypeSpec   *TypeSpec
	ValueTypeSpec *TypeSpec
}

// CodecStructSpec is a spec for a struct type.
type CodecStructSpec struct{}

// CodecTypedefSpec is a spec for a typedef type.
type CodecTypedefSpec struct {
	UnderlyingTypeSpec *TypeSpec
}

// TypeSpec is a union of all concrete specs.
type TypeSpec struct {
	CodecPrimitiveSpec *CodecPrimitiveSpec
	CodecEnumSpec      *CodecEnumSpec
	CodecSetSpec       *CodecSetSpec
	CodecListSpec      *CodecListSpec
	CodecMapSpec       *CodecMapSpec
	CodecTypedefSpec   *CodecTypedefSpec
	CodecStructSpec    *CodecStructSpec
}
