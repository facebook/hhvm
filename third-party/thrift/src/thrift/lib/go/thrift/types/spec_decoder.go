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

import (
	"errors"
	"fmt"
	"reflect"
)

// ReadStructSpec reads a struct from the given Decoder.
func ReadStructSpec(d Decoder, dstStruct Struct, spec *StructSpec) error {
	dstValue := reflect.ValueOf(dstStruct)
	dstConcreteValue := dstValue.Elem()
	// Concrete type of the struct
	structReflectType := dstConcreteValue.Type()

	if _, err := d.ReadStructBegin(); err != nil {
		return PrependError(fmt.Sprintf("%s read error: ", structReflectType.Name()), err)
	}

	for {
		fieldName, wireType, fieldID, err := d.ReadFieldBegin()
		if err != nil {
			return PrependError(fmt.Sprintf("%s field %d ('%s') read error: ", structReflectType.Name(), fieldID, fieldName), err)
		}

		if wireType == STOP {
			break
		}

		var fieldSpec *FieldSpec
		if fieldSpecIndex, found := spec.FieldSpecIDToIndex[fieldID]; found {
			fieldSpec = &spec.FieldSpecs[fieldSpecIndex]
		} else if fieldSpecIndex, found := spec.FieldSpecNameToIndex[fieldName]; found && fieldID == NO_FIELD_ID && fieldName != "" {
			fieldSpec = &spec.FieldSpecs[fieldSpecIndex]
			wireType = fieldSpec.WireType
		}

		var fieldReadErr error
		if fieldSpec == nil || wireType != fieldSpec.WireType {
			fieldReadErr = d.Skip(wireType)
		} else {
			fieldDstValue := dstConcreteValue.Field(fieldSpec.ReflectIndex)
			fieldReadErr = readFieldSpec(d, fieldDstValue, fieldSpec)
		}

		if fieldReadErr != nil {
			return fieldReadErr
		}

		if err := d.ReadFieldEnd(); err != nil {
			return err
		}
	}

	if err := d.ReadStructEnd(); err != nil {
		return PrependError(fmt.Sprintf("%s read struct end error: ", structReflectType.Name()), err)
	}

	return nil
}

// ReadTypeSpec reads a value from the given decoder according to the given TypeSpec.
func ReadTypeSpec(d Decoder, dstValue reflect.Value, spec *TypeSpec) error {
	switch {
	case spec.CodecPrimitiveSpec != nil:
		return readCodecPrimitiveSpec(d, dstValue, spec.CodecPrimitiveSpec)
	case spec.CodecEnumSpec != nil:
		return readCodecEnumSpec(d, dstValue, spec.CodecEnumSpec)
	case spec.CodecSetSpec != nil:
		return readCodecSetSpec(d, dstValue, spec.CodecSetSpec)
	case spec.CodecListSpec != nil:
		return readCodecListSpec(d, dstValue, spec.CodecListSpec)
	case spec.CodecMapSpec != nil:
		return readCodecMapSpec(d, dstValue, spec.CodecMapSpec)
	case spec.CodecTypedefSpec != nil:
		return readCodecTypedefSpec(d, dstValue, spec.CodecTypedefSpec)
	case spec.CodecStructSpec != nil:
		return readCodecStructSpec(d, dstValue, spec.CodecStructSpec)
	default:
		// NOTE: this error is impossible in practice.
		return errors.New("no codec spec - empty union")
	}
}

func readCodecPrimitiveSpec(d Decoder, dstValue reflect.Value, spec *CodecPrimitiveSpec) error {
	var readErr error

	switch spec.PrimitiveType {
	case CODEC_PRIMITIVE_TYPE_BYTE:
		var byteValue byte
		byteValue, err := d.ReadByte()
		value := int8(byteValue)
		dstValue.SetInt(int64(value))
		readErr = err
	case CODEC_PRIMITIVE_TYPE_BOOL:
		value, err := d.ReadBool()
		dstValue.SetBool(value)
		readErr = err
	case CODEC_PRIMITIVE_TYPE_I16:
		value, err := d.ReadI16()
		dstValue.SetInt(int64(value))
		readErr = err
	case CODEC_PRIMITIVE_TYPE_I32:
		value, err := d.ReadI32()
		dstValue.SetInt(int64(value))
		readErr = err
	case CODEC_PRIMITIVE_TYPE_I64:
		value, err := d.ReadI64()
		dstValue.SetInt(value)
		readErr = err
	case CODEC_PRIMITIVE_TYPE_FLOAT:
		value, err := d.ReadFloat()
		dstValue.SetFloat(float64(value))
		readErr = err
	case CODEC_PRIMITIVE_TYPE_DOUBLE:
		value, err := d.ReadDouble()
		dstValue.SetFloat(value)
		readErr = err
	case CODEC_PRIMITIVE_TYPE_BINARY:
		value, err := d.ReadBinary()
		dstValue.SetBytes(value)
		readErr = err
	case CODEC_PRIMITIVE_TYPE_STRING:
		value, err := d.ReadString()
		dstValue.SetString(value)
		readErr = err
	default:
		// NOTE: this error is impossible in practice.
		return errors.New("unknown primitive type")
	}

	return readErr
}

func readCodecEnumSpec(d Decoder, dstValue reflect.Value, _ *CodecEnumSpec) error {
	intValue, readErr := d.ReadI32()
	if readErr != nil {
		return readErr
	}

	dstValue.SetInt(int64(intValue))
	return nil
}

func readCodecSetSpec(d Decoder, dstValue reflect.Value, spec *CodecSetSpec) error {
	_ /* elemType */, size, err := d.ReadSetBegin()
	if err != nil {
		return PrependError("error reading set begin: ", err)
	}

	// NOTE: the 'dstValue' slice may not be empty, because of
	// Thrift field defaults. It may also be nil (zero value).
	// So we set it to a fresh empty slice.
	dstSlice := reflect.MakeSlice(dstValue.Type(), max(size, 0), max(size, 0))
	if size >= 0 { // Known size
		for i := range size {
			err := ReadTypeSpec(d, dstSlice.Index(i), spec.ElementTypeSpec)
			if err != nil {
				return err
			}
		}
	} else { // Unknown size
		for {
			elem := reflect.New(dstValue.Type().Elem())
			err := ReadTypeSpec(d, elem.Elem(), spec.ElementTypeSpec)
			if err != nil {
				break
			}
			dstSlice = reflect.Append(dstSlice, elem.Elem())
		}
	}

	dstValue.Set(dstSlice)

	if err := d.ReadSetEnd(); err != nil {
		return PrependError("error reading set end: ", err)
	}

	return nil
}

func readCodecListSpec(d Decoder, dstValue reflect.Value, spec *CodecListSpec) error {
	_ /* elemType */, size, err := d.ReadListBegin()
	if err != nil {
		return PrependError("error reading list begin: ", err)
	}

	// NOTE: the 'dstValue' slice may not be empty, because of
	// Thrift field defaults. It may also be nil (zero value).
	// So we set it to a fresh empty slice.
	dstSlice := reflect.MakeSlice(dstValue.Type(), max(size, 0), max(size, 0))
	if size >= 0 { // Known size
		for i := range size {
			err := ReadTypeSpec(d, dstSlice.Index(i), spec.ElementTypeSpec)
			if err != nil {
				return err
			}
		}
	} else { // Unknown size
		for {
			elem := reflect.New(dstValue.Type().Elem())
			err := ReadTypeSpec(d, elem.Elem(), spec.ElementTypeSpec)
			if err != nil {
				break
			}
			dstSlice = reflect.Append(dstSlice, elem.Elem())
		}
	}

	dstValue.Set(dstSlice)

	if err := d.ReadListEnd(); err != nil {
		return PrependError("error reading list end: ", err)
	}

	return nil
}

func readCodecMapSpec(d Decoder, dstValue reflect.Value, spec *CodecMapSpec) error {
	_ /* keyType */, _ /* valueType */, size, err := d.ReadMapBegin()
	if err != nil {
		return PrependError("error reading map begin: ", err)
	}

	// NOTE: the 'dstValue' map might not be empty, because of
	// Thrift field defaults. It may also be nil (zero value).
	// So we set it to a fresh empty map.
	dstValue.Set(reflect.MakeMapWithSize(dstValue.Type(), max(size, 0)))

	keyReflectType := dstValue.Type().Key()
	valReflectType := dstValue.Type().Elem()

	// Map keys need special handling because they have comparability requirement
	// for keys and key types may be a bit different than in other scenarios.
	underlyingKeyReflectType := keyReflectType
	if keyReflectType.Kind() == reflect.Pointer {
		underlyingKeyReflectType = keyReflectType.Elem()
	}
	isComparable := underlyingKeyReflectType.Comparable()
	isStruct := underlyingKeyReflectType.Kind() == reflect.Struct

	passedKeyReflectType := keyReflectType
	if isComparable && isStruct {
		// 'keyReflectType' is a concrete struct (i.e. not a struct pointer)
		// We must "take address" before passing recursively downstream, because
		// 'readCodecStructSpec' expects a pointer to a struct, not a concrete struct.
		passedKeyReflectType = reflect.PointerTo(keyReflectType)
	} else if !isComparable && !isStruct {
		// 'keyReflectType' is a pointer to a non-struct type.
		// We must "dereference" before passing recursively downstream,
		// because downstream 'read' functions expect concrete values.
		passedKeyReflectType = keyReflectType.Elem()
	}

	// Temporary slices to hold keys and values. This is a performance optimization.
	// Rather than calling reflect.New() for each key and value, we create slices
	// which create zero values for us under-the-hood. This is much faster.
	keysSlice := reflect.MakeSlice(reflect.SliceOf(passedKeyReflectType), max(size, 0), max(size, 0))
	valuesSlice := reflect.MakeSlice(reflect.SliceOf(valReflectType), max(size, 0), max(size, 0))

	// Read keys and values into temporary slices.
	if size >= 0 {
		for i := range size {
			passedKeyReflectValue := keysSlice.Index(i)
			err := ReadTypeSpec(d, passedKeyReflectValue, spec.KeyTypeSpec)
			if err != nil {
				return err
			}
			valueReflectValue := valuesSlice.Index(i)
			err = ReadTypeSpec(d, valueReflectValue, spec.ValueTypeSpec)
			if err != nil {
				return err
			}

			assignedKeyReflectValue := passedKeyReflectValue
			if isComparable && isStruct {
				// See comments earlier in this function. This is the same logic in reverse.
				assignedKeyReflectValue = passedKeyReflectValue.Elem()
			} else if !isComparable && !isStruct {
				// See comments earlier in this function. This is the same logic in reverse.
				assignedKeyReflectValue = passedKeyReflectValue.Addr()
			}

			dstValue.SetMapIndex(assignedKeyReflectValue, valueReflectValue)
		}
	} else {
		for {
			keyElem := reflect.New(passedKeyReflectType)
			valueElem := reflect.New(valReflectType)

			passedKeyReflectValue := keyElem.Elem()
			err := ReadTypeSpec(d, passedKeyReflectValue, spec.KeyTypeSpec)
			if err != nil {
				break
			}
			valueReflectValue := valueElem.Elem()
			err = ReadTypeSpec(d, valueReflectValue, spec.ValueTypeSpec)
			if err != nil {
				return err
			}

			assignedKeyReflectValue := passedKeyReflectValue
			if isComparable && isStruct {
				// See comments earlier in this function. This is the same logic in reverse.
				assignedKeyReflectValue = passedKeyReflectValue.Elem()
			} else if !isComparable && !isStruct {
				// See comments earlier in this function. This is the same logic in reverse.
				assignedKeyReflectValue = passedKeyReflectValue.Addr()
			}

			dstValue.SetMapIndex(assignedKeyReflectValue, valueReflectValue)
		}
	}

	if err := d.ReadMapEnd(); err != nil {
		return PrependError("error reading map end: ", err)
	}

	return nil
}

func readCodecTypedefSpec(d Decoder, dstValue reflect.Value, spec *CodecTypedefSpec) error {
	// Pass-through using the underlying type spec.
	return ReadTypeSpec(d, dstValue, spec.UnderlyingTypeSpec)
}

func readCodecStructSpec(d Decoder, dstValue reflect.Value, spec *CodecStructSpec) error {
	// Create a fresh instance of the underlying struct.
	// Note that Thrift-defined field defaults will also be set.
	// This is exactly what we want.
	newStruct := spec.NewFunc()
	readErr := newStruct.Read(d)
	dstValue.Set(reflect.ValueOf(newStruct))
	return readErr
}

func readFieldSpec(d Decoder, fieldDstValue reflect.Value, fieldSpec *FieldSpec) error {
	isPointer := fieldDstValue.Kind() == reflect.Pointer
	underlyingFieldReflectType := fieldDstValue.Type()
	if isPointer {
		underlyingFieldReflectType = fieldDstValue.Type().Elem()
	}
	isStruct := underlyingFieldReflectType.Kind() == reflect.Struct

	passedReflectValue := fieldDstValue
	if isPointer && !isStruct {
		// We are dealing with a non-struct pointer, which means this is
		// an optional field, or a union field. Downstream functions require
		// concrete reflect values rather than pointers for all types except
		// struct. So, we need to perform a "dereference" to make things proper.
		// Equivalent: `arv = new(UnderlyingType); prv := *arv;`
		fieldDstValue.Set(reflect.New(underlyingFieldReflectType))
		passedReflectValue = fieldDstValue.Elem()
	}

	fieldReadErr := ReadTypeSpec(d, passedReflectValue, fieldSpec.ValueTypeSpec)
	if fieldReadErr != nil {
		return fieldReadErr
	}

	return nil
}
