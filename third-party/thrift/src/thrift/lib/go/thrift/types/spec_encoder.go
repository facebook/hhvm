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

// WriteStructSpec writes a struct to the given Encoder.
func WriteStructSpec(d Encoder, srcStruct Struct, spec *StructSpec) error {
	srcValue := reflect.ValueOf(srcStruct)
	srcConcreteValue := srcValue.Elem()
	// Concrete type of the struct
	structReflectType := srcConcreteValue.Type()

	if err := d.WriteStructBegin(spec.Name); err != nil {
		return PrependError(fmt.Sprintf("%s write error: ", structReflectType.Name()), err)
	}

	for _, fieldSpec := range spec.FieldSpecs {
		fieldSrcValue := srcConcreteValue.Field(fieldSpec.ReflectIndex)

		if fieldSpec.MustBeSetToSerialize && fieldSrcValue.IsNil() {
			continue
		}

		err := d.WriteFieldBegin(fieldSpec.Name, fieldSpec.WireType, fieldSpec.ID)
		if err != nil {
			return PrependError(fmt.Sprintf("%s field %d write error: ", structReflectType.Name(), fieldSpec.ID), err)
		}

		fieldWriteErr := writeFieldSpec(d, fieldSrcValue, &fieldSpec)
		if fieldWriteErr != nil {
			return fieldWriteErr
		}

		if err := d.WriteFieldEnd(); err != nil {
			return err
		}
	}

	if err := d.WriteFieldStop(); err != nil {
		return PrependError(fmt.Sprintf("%T write field stop error: ", structReflectType.Name()), err)
	}

	if err := d.WriteStructEnd(); err != nil {
		return PrependError(fmt.Sprintf("%s write struct end error: ", structReflectType.Name()), err)
	}

	return nil
}

func writeTypeSpec(d Encoder, srcValue reflect.Value, spec *TypeSpec) error {
	switch {
	case spec.CodecPrimitiveSpec != nil:
		return writeCodecPrimitiveSpec(d, srcValue, spec.CodecPrimitiveSpec)
	case spec.CodecEnumSpec != nil:
		return writeCodecEnumSpec(d, srcValue, spec.CodecEnumSpec)
	case spec.CodecSetSpec != nil:
		return writeCodecSetSpec(d, srcValue, spec.CodecSetSpec)
	case spec.CodecListSpec != nil:
		return writeCodecListSpec(d, srcValue, spec.CodecListSpec)
	case spec.CodecMapSpec != nil:
		return writeCodecMapSpec(d, srcValue, spec.CodecMapSpec)
	case spec.CodecTypedefSpec != nil:
		return writeCodecTypedefSpec(d, srcValue, spec.CodecTypedefSpec)
	case spec.CodecStructSpec != nil:
		return writeCodecStructSpec(d, srcValue, spec.CodecStructSpec)
	default:
		// NOTE: this error is impossible in practice.
		return errors.New("no codec spec - empty union")
	}
}

func writeCodecPrimitiveSpec(d Encoder, srcValue reflect.Value, spec *CodecPrimitiveSpec) error {
	var writeErr error

	switch spec.PrimitiveType {
	case CODEC_PRIMITIVE_TYPE_BYTE:
		value := byte(srcValue.Int())
		writeErr = d.WriteByte(value)
	case CODEC_PRIMITIVE_TYPE_BOOL:
		value := srcValue.Bool()
		writeErr = d.WriteBool(value)
	case CODEC_PRIMITIVE_TYPE_I16:
		value := int16(srcValue.Int())
		writeErr = d.WriteI16(value)
	case CODEC_PRIMITIVE_TYPE_I32:
		value := int32(srcValue.Int())
		writeErr = d.WriteI32(value)
	case CODEC_PRIMITIVE_TYPE_I64:
		value := srcValue.Int()
		writeErr = d.WriteI64(value)
	case CODEC_PRIMITIVE_TYPE_FLOAT:
		value := float32(srcValue.Float())
		writeErr = d.WriteFloat(value)
	case CODEC_PRIMITIVE_TYPE_DOUBLE:
		value := srcValue.Float()
		writeErr = d.WriteDouble(value)
	case CODEC_PRIMITIVE_TYPE_BINARY:
		value := srcValue.Bytes()
		writeErr = d.WriteBinary(value)
	case CODEC_PRIMITIVE_TYPE_STRING:
		value := srcValue.String()
		writeErr = d.WriteString(value)
	default:
		// NOTE: this error is impossible in practice.
		return errors.New("unknown primitive type")
	}

	return writeErr
}

func writeCodecEnumSpec(d Encoder, srcValue reflect.Value, _ *CodecEnumSpec) error {
	value := int32(srcValue.Int())
	return d.WriteI32(value)
}

func writeCodecSetSpec(d Encoder, srcValue reflect.Value, spec *CodecSetSpec) error {
	err := d.WriteSetBegin(spec.ElementWireType, srcValue.Len())
	if err != nil {
		return PrependError("error writing set begin: ", err)
	}

	for i := range srcValue.Len() {
		err := writeTypeSpec(d, srcValue.Index(i), spec.ElementTypeSpec)
		if err != nil {
			return err
		}
	}

	if err := d.WriteSetEnd(); err != nil {
		return PrependError("error writing set end: ", err)
	}

	return nil
}

func writeCodecListSpec(d Encoder, srcValue reflect.Value, spec *CodecListSpec) error {
	err := d.WriteListBegin(spec.ElementWireType, srcValue.Len())
	if err != nil {
		return PrependError("error writing list begin: ", err)
	}

	for i := range srcValue.Len() {
		err := writeTypeSpec(d, srcValue.Index(i), spec.ElementTypeSpec)
		if err != nil {
			return err
		}
	}

	if err := d.WriteListEnd(); err != nil {
		return PrependError("error writing list end: ", err)
	}

	return nil
}

func writeCodecMapSpec(d Encoder, srcValue reflect.Value, spec *CodecMapSpec) error {
	err := d.WriteMapBegin(spec.KeyWireType, spec.ValueWireType, srcValue.Len())
	if err != nil {
		return PrependError("error writing map begin: ", err)
	}

	keyReflectType := srcValue.Type().Key()

	// Map keys need special handling because they have comparability requirement
	// for keys and key types may be a bit different than in other scenarios.
	underlyingKeyReflectType := keyReflectType
	if keyReflectType.Kind() == reflect.Pointer {
		underlyingKeyReflectType = keyReflectType.Elem()
	}
	isComparable := underlyingKeyReflectType.Comparable()
	isStruct := underlyingKeyReflectType.Kind() == reflect.Struct

	iter := srcValue.MapRange()
	for iter.Next() {
		// Reflect value that will be read into recursively downstream.
		passedKeyReflectValue := iter.Key()
		if isComparable && isStruct {
			// 'keyReflectType' is a concrete struct (i.e. not a struct pointer)
			// We must "take address" before passing recursively downstream, because
			// 'readCodecStructSpec' expects a pointer to a struct, not a concrete struct.
			passedKeyReflectValue = reflect.New(keyReflectType)
			passedKeyReflectValue.Elem().Set(iter.Key())
		} else if !isComparable && !isStruct {
			// 'keyReflectType' is a pointer to a non-struct type.
			// We must "dereference" before passing recursively downstream,
			// because downstream 'read' functions expect concrete values.
			passedKeyReflectValue = iter.Key().Elem()
		}

		err := writeTypeSpec(d, passedKeyReflectValue, spec.KeyTypeSpec)
		if err != nil {
			return err
		}

		err = writeTypeSpec(d, iter.Value(), spec.ValueTypeSpec)
		if err != nil {
			return err
		}
	}

	if err := d.WriteMapEnd(); err != nil {
		return PrependError("error writing map end: ", err)
	}

	return nil
}

func writeCodecTypedefSpec(d Encoder, srcValue reflect.Value, spec *CodecTypedefSpec) error {
	// Pass-through using the underlying type spec.
	return writeTypeSpec(d, srcValue, spec.UnderlyingTypeSpec)
}

func writeCodecStructSpec(d Encoder, srcValue reflect.Value, spec *CodecStructSpec) error {
	// Call "Write()" method on the struct.
	srcValueAsStruct := srcValue.Interface().(Struct)
	return srcValueAsStruct.Write(d)
}

func writeFieldSpec(d Encoder, fieldSrcValue reflect.Value, fieldSpec *FieldSpec) error {
	isPointer := fieldSrcValue.Kind() == reflect.Pointer
	underlyingFieldReflectType := fieldSrcValue.Type()
	if isPointer {
		underlyingFieldReflectType = fieldSrcValue.Type().Elem()
	}
	isStruct := underlyingFieldReflectType.Kind() == reflect.Struct

	passedReflectValue := fieldSrcValue
	if isPointer && !isStruct {
		// Because downstream functions expect concrete values, we must dereference
		// any non-struct pointers before passing them recursively downstream.
		// Equivalent: `prv = *fsv`
		passedReflectValue = fieldSrcValue.Elem()
	}

	fieldWriteErr := writeTypeSpec(d, passedReflectValue, fieldSpec.ValueTypeSpec)
	if fieldWriteErr != nil {
		return fieldWriteErr
	}

	return nil
}
