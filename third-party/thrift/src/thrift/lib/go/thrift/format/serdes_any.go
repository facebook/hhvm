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
	"bytes"
	"errors"
	"reflect"

	thriftany "thrift/lib/thrift/any"
	thriftstandard "thrift/lib/thrift/standard"
	thrifttyperep "thrift/lib/thrift/type_rep"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

// DecodeAny takes an "Any" struct and decodes it.
// The resulting 'any' object (if no error) can be
// type-asserted to a concrete type by the caller.
func DecodeAny(anyStruct *thriftany.Any) (any, error) {
	if !anyStruct.IsSetType() {
		return nil, errors.New("type is not set")
	}
	if !anyStruct.IsSetProtocol() {
		return nil, errors.New("protocol is not set")
	}

	anyType := anyStruct.GetType()
	if !anyType.IsSetName() {
		return nil, errors.New("name is not set")
	}

	anyProtocol := anyStruct.GetProtocol()
	if !anyProtocol.IsSetStandard() {
		// TODO: extend protocol support (e.g. compression)
		return nil, errors.New("only standard protocol is supported")
	}

	reader := bytes.NewBuffer(anyStruct.GetData())
	var decoder types.Decoder
	switch anyProtocol.GetStandard() {
	case thriftstandard.StandardProtocol_Binary:
		decoder = NewBinaryFormat(reader)
	case thriftstandard.StandardProtocol_Compact:
		decoder = NewCompactFormat(reader)
	case thriftstandard.StandardProtocol_Json:
		decoder = NewCompactJSONFormat(reader)
	case thriftstandard.StandardProtocol_SimpleJson:
		decoder = NewSimpleJSONFormat(reader)
	default:
		return nil, errors.New("unsupported standard protocol")
	}

	typeSpec, dstValue, err := getTypeSpec(anyType)
	if err != nil {
		return nil, err
	}

	err = types.ReadTypeSpec(decoder, dstValue, typeSpec)
	if err != nil {
		return nil, err
	}
	return dstValue.Interface(), nil
}

func getTypeSpec(typeStruct *thrifttyperep.TypeStruct) (*types.TypeSpec, reflect.Value, error) {
	typeName := typeStruct.GetName()

	var typeSpec *types.TypeSpec
	var dstValue reflect.Value
	var err error
	switch {
	case typeName.IsSetBoolType():
		typeSpec = &types.TypeSpec{
			CodecPrimitiveSpec: &types.CodecPrimitiveSpec{
				PrimitiveType: types.CODEC_PRIMITIVE_TYPE_BOOL,
			},
		}
		dstValue = reflect.ValueOf(bool(false))
	case typeName.IsSetByteType():
		typeSpec = &types.TypeSpec{
			CodecPrimitiveSpec: &types.CodecPrimitiveSpec{
				PrimitiveType: types.CODEC_PRIMITIVE_TYPE_BYTE,
			},
		}
		dstValue = reflect.ValueOf(int8(0))
	case typeName.IsSetI16Type():
		typeSpec = &types.TypeSpec{
			CodecPrimitiveSpec: &types.CodecPrimitiveSpec{
				PrimitiveType: types.CODEC_PRIMITIVE_TYPE_I16,
			},
		}
		dstValue = reflect.ValueOf(int16(0))
	case typeName.IsSetI32Type():
		typeSpec = &types.TypeSpec{
			CodecPrimitiveSpec: &types.CodecPrimitiveSpec{
				PrimitiveType: types.CODEC_PRIMITIVE_TYPE_I32,
			},
		}
		dstValue = reflect.ValueOf(int32(0))
	case typeName.IsSetI64Type():
		typeSpec = &types.TypeSpec{
			CodecPrimitiveSpec: &types.CodecPrimitiveSpec{
				PrimitiveType: types.CODEC_PRIMITIVE_TYPE_I64,
			},
		}
		dstValue = reflect.ValueOf(int64(0))
	case typeName.IsSetFloatType():
		typeSpec = &types.TypeSpec{
			CodecPrimitiveSpec: &types.CodecPrimitiveSpec{
				PrimitiveType: types.CODEC_PRIMITIVE_TYPE_FLOAT,
			},
		}
		dstValue = reflect.ValueOf(float32(0))
	case typeName.IsSetDoubleType():
		typeSpec = &types.TypeSpec{
			CodecPrimitiveSpec: &types.CodecPrimitiveSpec{
				PrimitiveType: types.CODEC_PRIMITIVE_TYPE_DOUBLE,
			},
		}
		dstValue = reflect.ValueOf(float64(0))
	case typeName.IsSetStringType():
		typeSpec = &types.TypeSpec{
			CodecPrimitiveSpec: &types.CodecPrimitiveSpec{
				PrimitiveType: types.CODEC_PRIMITIVE_TYPE_STRING,
			},
		}
		dstValue = reflect.ValueOf("")
	case typeName.IsSetBinaryType():
		typeSpec = &types.TypeSpec{
			CodecPrimitiveSpec: &types.CodecPrimitiveSpec{
				PrimitiveType: types.CODEC_PRIMITIVE_TYPE_BINARY,
			},
		}
		dstValue = reflect.ValueOf([]byte{})
	case typeName.IsSetEnumType():
		typeSpec, err = getTypeSpecFromTypeURI(typeName.GetEnumType())
		if err != nil {
			return nil, reflect.Value{}, err
		}
		dstValue = reflect.ValueOf(typeSpec.CodecEnumSpec.NewFunc())
	case typeName.IsSetTypedefType():
		typeSpec, err = getTypeSpecFromTypeURI(typeName.GetTypedefType())
		if err != nil {
			return nil, reflect.Value{}, err
		}
		typedefPtr := typeSpec.CodecTypedefSpec.NewFunc()
		dstValue = reflect.ValueOf(&typedefPtr).Elem()
	case typeName.IsSetStructType():
		typeSpec, err = getTypeSpecFromTypeURI(typeName.GetStructType())
		if err != nil {
			return nil, reflect.Value{}, err
		}
		structPtr := typeSpec.CodecStructSpec.NewFunc()
		dstValue = reflect.ValueOf(&structPtr).Elem()
	case typeName.IsSetUnionType():
		typeSpec, err = getTypeSpecFromTypeURI(typeName.GetUnionType())
		if err != nil {
			return nil, reflect.Value{}, err
		}
		structPtr := typeSpec.CodecStructSpec.NewFunc()
		dstValue = reflect.ValueOf(&structPtr).Elem()
	case typeName.IsSetExceptionType():
		typeSpec, err = getTypeSpecFromTypeURI(typeName.GetExceptionType())
		if err != nil {
			return nil, reflect.Value{}, err
		}
		structPtr := typeSpec.CodecStructSpec.NewFunc()
		dstValue = reflect.ValueOf(&structPtr).Elem()
	case typeName.IsSetListType():
		if len(typeStruct.GetParams()) != 1 {
			return nil, reflect.Value{}, errors.New("list type must have exactly one parameter")
		}
		elemTypeSpec, elemDstValue, err := getTypeSpec(typeStruct.GetParams()[0])
		if err != nil {
			return nil, reflect.Value{}, err
		}
		typeSpec = &types.TypeSpec{
			CodecListSpec: &types.CodecListSpec{
				ElementTypeSpec: elemTypeSpec,
			},
		}
		dstValue = reflect.MakeSlice(elemDstValue.Type(), 0, 0)
	case typeName.IsSetSetType():
		if len(typeStruct.GetParams()) != 1 {
			return nil, reflect.Value{}, errors.New("set type must have exactly one parameter")
		}
		elemTypeSpec, elemDstValue, err := getTypeSpec(typeStruct.GetParams()[0])
		if err != nil {
			return nil, reflect.Value{}, err
		}
		typeSpec = &types.TypeSpec{
			CodecSetSpec: &types.CodecSetSpec{
				ElementTypeSpec: elemTypeSpec,
			},
		}
		dstValue = reflect.MakeSlice(elemDstValue.Type(), 0, 0)
	case typeName.IsSetMapType():
		if len(typeStruct.GetParams()) != 2 {
			return nil, reflect.Value{}, errors.New("map type must have exactly two parameters")
		}
		keyTypeSpec, keyDstValue, err := getTypeSpec(typeStruct.GetParams()[0])
		if err != nil {
			return nil, reflect.Value{}, err
		}
		valTypeSpec, valDstValue, err := getTypeSpec(typeStruct.GetParams()[1])
		if err != nil {
			return nil, reflect.Value{}, err
		}
		typeSpec = &types.TypeSpec{
			CodecMapSpec: &types.CodecMapSpec{
				KeyTypeSpec:   keyTypeSpec,
				ValueTypeSpec: valTypeSpec,
			},
		}
		// TODO: edge case - comparable/struct
		dstValue = reflect.MakeMap(reflect.MapOf(keyDstValue.Type(), valDstValue.Type()))
	default:
		return nil, reflect.Value{}, errors.New("unknown type name")
	}
	return typeSpec, dstValue, nil
}

func getTypeSpecFromTypeURI(typeURI *thriftstandard.TypeUri) (*types.TypeSpec, error) {
	switch {
	case typeURI.IsSetUri():
		typeSpec, ok := types.InternalTypeRegistryGetFromURI(typeURI.GetUri())
		if !ok {
			return nil, errors.New("type not found in registry")
		}
		return typeSpec, nil
	case typeURI.IsSetTypeHashPrefixSha2_256():
		typeSpec, ok := types.InternalTypeRegistryGetFromHash(typeURI.GetTypeHashPrefixSha2_256())
		if !ok {
			return nil, errors.New("type not found in registry")
		}
		return typeSpec, nil
	case typeURI.IsSetScopedName():
		return nil, errors.New("scoped name is not supported")
	case typeURI.IsSetDefinitionKey():
		return nil, errors.New("definition key is not supported")
	default:
		return nil, errors.New("unknown type uri")
	}
}
