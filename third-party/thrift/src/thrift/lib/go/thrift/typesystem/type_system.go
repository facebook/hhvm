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

// Package typesystem defines runtime interfaces for Thrift type reflection.
package typesystem

import (
	"fmt"

	typeid "thrift/lib/thrift/type_id"
	typesystemgen "thrift/lib/thrift/type_system"
)

// TypeSystem is the runtime lookup interface for Thrift type definitions.
type TypeSystem interface {
	Get(uri string) (TypeDefinition, bool)
	Contains(uri string) bool
	KnownURIs() []string
	Resolve(typeID *typeid.TypeId) (TypeRef, error)
	ToSerializableTypeSystem() (*typesystemgen.SerializableTypeSystem, error)
}

// TypeDefinition is a user-defined type node in a TypeSystem.
type TypeDefinition interface {
	Name() string
	URI() string
	TypeKind() TypeRefKind
}

// TypedefDefinition is a typedef-like type definition with a target type.
type TypedefDefinition interface {
	TypeDefinition
	TargetType() (TypeRef, error)
}

// TypeRefKind identifies a resolved thrift type.
type TypeRefKind int

const (
	TypeRefKindBool TypeRefKind = iota + 1
	TypeRefKindByte
	TypeRefKindI16
	TypeRefKindI32
	TypeRefKindI64
	TypeRefKindFloat
	TypeRefKindDouble
	TypeRefKindString
	TypeRefKindBinary
	TypeRefKindAny
	TypeRefKindList
	TypeRefKindSet
	TypeRefKindMap
	TypeRefKindStruct
	TypeRefKindUnion
	TypeRefKindException
	TypeRefKindEnum
	TypeRefKindTypedef
	TypeRefKindOpaqueAlias
)

func (k TypeRefKind) String() string {
	switch k {
	case TypeRefKindBool:
		return "bool"
	case TypeRefKindByte:
		return "byte"
	case TypeRefKindI16:
		return "i16"
	case TypeRefKindI32:
		return "i32"
	case TypeRefKindI64:
		return "i64"
	case TypeRefKindFloat:
		return "float"
	case TypeRefKindDouble:
		return "double"
	case TypeRefKindString:
		return "string"
	case TypeRefKindBinary:
		return "binary"
	case TypeRefKindAny:
		return "any"
	case TypeRefKindList:
		return "list"
	case TypeRefKindSet:
		return "set"
	case TypeRefKindMap:
		return "map"
	case TypeRefKindStruct:
		return "struct"
	case TypeRefKindUnion:
		return "union"
	case TypeRefKindException:
		return "exception"
	case TypeRefKindEnum:
		return "enum"
	case TypeRefKindTypedef:
		return "typedef"
	case TypeRefKindOpaqueAlias:
		return "opaque_alias"
	default:
		return "invalid"
	}
}

// TypeRef is a resolved primitive, container, or user-defined type reference.
type TypeRef struct {
	kind       TypeRefKind
	definition TypeDefinition
	element    *TypeRef
	key        *TypeRef
	value      *TypeRef
}

func Primitive(kind TypeRefKind) TypeRef {
	return TypeRef{kind: kind}
}

func UserDefined(kind TypeRefKind, definition TypeDefinition) TypeRef {
	return TypeRef{kind: kind, definition: definition}
}

func ListOf(element TypeRef) TypeRef {
	return TypeRef{kind: TypeRefKindList, element: new(element)}
}

func SetOf(element TypeRef) TypeRef {
	return TypeRef{kind: TypeRefKindSet, element: new(element)}
}

func MapOf(key TypeRef, value TypeRef) TypeRef {
	return TypeRef{kind: TypeRefKindMap, key: new(key), value: new(value)}
}

func (r TypeRef) Kind() TypeRefKind {
	return r.kind
}

func (r TypeRef) IsPrimitive() bool {
	switch r.kind {
	case TypeRefKindBool,
		TypeRefKindByte,
		TypeRefKindI16,
		TypeRefKindI32,
		TypeRefKindI64,
		TypeRefKindFloat,
		TypeRefKindDouble,
		TypeRefKindString,
		TypeRefKindBinary,
		TypeRefKindAny:
		return true
	default:
		return false
	}
}

func (r TypeRef) IsContainer() bool {
	switch r.kind {
	case TypeRefKindList, TypeRefKindSet, TypeRefKindMap:
		return true
	default:
		return false
	}
}

func (r TypeRef) IsStruct() bool {
	return r.kind == TypeRefKindStruct
}

func (r TypeRef) IsUnion() bool {
	return r.kind == TypeRefKindUnion
}

func (r TypeRef) IsException() bool {
	return r.kind == TypeRefKindException
}

func (r TypeRef) IsStructured() bool {
	return r.IsStruct() || r.IsUnion() || r.IsException()
}

func (r TypeRef) IsEnum() bool {
	return r.kind == TypeRefKindEnum
}

func (r TypeRef) IsTypedef() bool {
	return r.kind == TypeRefKindTypedef || r.kind == TypeRefKindOpaqueAlias
}

func (r TypeRef) IsList() bool {
	return r.kind == TypeRefKindList
}

func (r TypeRef) IsSet() bool {
	return r.kind == TypeRefKindSet
}

func (r TypeRef) IsMap() bool {
	return r.kind == TypeRefKindMap
}

func (r TypeRef) Definition() (TypeDefinition, error) {
	if r.definition == nil {
		return nil, fmt.Errorf("type ref %s has no user-defined definition", r.kind)
	}
	return r.definition, nil
}

func (r TypeRef) Element() (TypeRef, error) {
	if r.element == nil {
		return TypeRef{}, fmt.Errorf("type ref %s has no element type", r.kind)
	}
	return *r.element, nil
}

func (r TypeRef) Key() (TypeRef, error) {
	if r.key == nil {
		return TypeRef{}, fmt.Errorf("type ref %s has no key type", r.kind)
	}
	return *r.key, nil
}

func (r TypeRef) Value() (TypeRef, error) {
	if r.value == nil {
		return TypeRef{}, fmt.Errorf("type ref %s has no value type", r.kind)
	}
	return *r.value, nil
}

func (r TypeRef) TypeID() (*typeid.TypeId, error) {
	switch r.kind {
	case TypeRefKindBool:
		return &typeid.TypeId{BoolType: &typeid.BoolTypeId{}}, nil
	case TypeRefKindByte:
		return &typeid.TypeId{ByteType: &typeid.ByteTypeId{}}, nil
	case TypeRefKindI16:
		return &typeid.TypeId{I16Type: &typeid.I16TypeId{}}, nil
	case TypeRefKindI32:
		return &typeid.TypeId{I32Type: &typeid.I32TypeId{}}, nil
	case TypeRefKindI64:
		return &typeid.TypeId{I64Type: &typeid.I64TypeId{}}, nil
	case TypeRefKindFloat:
		return &typeid.TypeId{FloatType: &typeid.FloatTypeId{}}, nil
	case TypeRefKindDouble:
		return &typeid.TypeId{DoubleType: &typeid.DoubleTypeId{}}, nil
	case TypeRefKindString:
		return &typeid.TypeId{StringType: &typeid.StringTypeId{}}, nil
	case TypeRefKindBinary:
		return &typeid.TypeId{BinaryType: &typeid.BinaryTypeId{}}, nil
	case TypeRefKindAny:
		return &typeid.TypeId{AnyType: &typeid.AnyTypeId{}}, nil
	case TypeRefKindList:
		element, err := r.Element()
		if err != nil {
			return nil, err
		}
		elementType, err := element.TypeID()
		if err != nil {
			return nil, err
		}
		return &typeid.TypeId{ListType: &typeid.ListTypeId{ElementType: elementType}}, nil
	case TypeRefKindSet:
		element, err := r.Element()
		if err != nil {
			return nil, err
		}
		elementType, err := element.TypeID()
		if err != nil {
			return nil, err
		}
		return &typeid.TypeId{SetType: &typeid.SetTypeId{ElementType: elementType}}, nil
	case TypeRefKindMap:
		key, err := r.Key()
		if err != nil {
			return nil, err
		}
		keyType, err := key.TypeID()
		if err != nil {
			return nil, err
		}
		value, err := r.Value()
		if err != nil {
			return nil, err
		}
		valueType, err := value.TypeID()
		if err != nil {
			return nil, err
		}
		return &typeid.TypeId{MapType: &typeid.MapTypeId{KeyType: keyType, ValueType: valueType}}, nil
	case TypeRefKindStruct,
		TypeRefKindUnion,
		TypeRefKindException,
		TypeRefKindEnum,
		TypeRefKindTypedef,
		TypeRefKindOpaqueAlias:
		definition, err := r.Definition()
		if err != nil {
			return nil, err
		}
		uri := definition.URI()
		if uri == "" {
			return nil, fmt.Errorf("type ref %s definition has no URI", r.kind)
		}
		return &typeid.TypeId{UserDefinedType: new(typeid.Uri(uri))}, nil
	default:
		return nil, fmt.Errorf("type ref %s cannot be converted to TypeId", r.kind)
	}
}

func (r TypeRef) AsStruct() (TypeDefinition, error) {
	return r.definitionWithKind(TypeRefKindStruct)
}

func (r TypeRef) AsUnion() (TypeDefinition, error) {
	return r.definitionWithKind(TypeRefKindUnion)
}

func (r TypeRef) AsException() (TypeDefinition, error) {
	return r.definitionWithKind(TypeRefKindException)
}

func (r TypeRef) AsStructured() (TypeDefinition, error) {
	if !r.IsStructured() {
		return nil, fmt.Errorf("type ref %s is not structured", r.kind)
	}
	return r.Definition()
}

func (r TypeRef) AsEnum() (TypeDefinition, error) {
	return r.definitionWithKind(TypeRefKindEnum)
}

func (r TypeRef) AsTypedef() (TypedefDefinition, error) {
	if !r.IsTypedef() {
		return nil, fmt.Errorf("type ref %s is not typedef", r.kind)
	}
	definition, err := r.Definition()
	if err != nil {
		return nil, err
	}
	typedef, ok := definition.(TypedefDefinition)
	if !ok {
		return nil, fmt.Errorf("type ref %s definition does not expose a typedef target", r.kind)
	}
	return typedef, nil
}

func (r TypeRef) TrueType() (TypeRef, error) {
	return r.trueType(make(map[string]struct{}))
}

func (r TypeRef) trueType(seen map[string]struct{}) (TypeRef, error) {
	if !r.IsTypedef() {
		return r, nil
	}
	typedef, err := r.AsTypedef()
	if err != nil {
		return TypeRef{}, err
	}
	uri := typedef.URI()
	if uri == "" {
		return TypeRef{}, fmt.Errorf("type ref %s typedef definition has no URI", r.kind)
	}
	if _, ok := seen[uri]; ok {
		return TypeRef{}, fmt.Errorf("cyclic typedef chain detected at %q", uri)
	}
	seen[uri] = struct{}{}
	target, err := typedef.TargetType()
	if err != nil {
		return TypeRef{}, err
	}
	return target.trueType(seen)
}

func (r TypeRef) definitionWithKind(expected TypeRefKind) (TypeDefinition, error) {
	if r.kind != expected {
		return nil, fmt.Errorf("type ref %s is not %s", r.kind, expected)
	}
	return r.Definition()
}
