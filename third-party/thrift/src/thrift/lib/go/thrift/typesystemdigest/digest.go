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

// Package typesystemdigest computes the canonical TypeSystem SHA-256 digest.
//
// The byte format matches the C++, Python, and Rust implementations:
// little-endian primitives, top-level types sorted by URI, fields sorted by
// field id, enum values sorted by datum, and unordered annotations/sets/maps
// sorted by per-element subdigest.
package typesystemdigest

import (
	"bytes"
	"cmp"
	"crypto/sha256"
	"encoding/binary"
	"fmt"
	"hash"
	"math"
	"slices"

	record "thrift/lib/thrift/record"
	typeid "thrift/lib/thrift/type_id"
	typesystem "thrift/lib/thrift/type_system"
)

const digestVersion byte = 2

// Mode controls which parts of a type system are hashed.
type Mode int

const (
	// ModeFull hashes definitions, annotations, and custom default values.
	ModeFull Mode = iota
	// ModeStructural hashes only wire-compatible structure.
	ModeStructural
)

// Digest computes the canonical 32-byte SHA-256 digest of a SerializableTypeSystem.
func Digest(typeSystem *typesystem.SerializableTypeSystem) ([]byte, error) {
	return digest(serializableTypeSystemNode{value: typeSystem})
}

// DigestWithMode computes the canonical 32-byte SHA-256 digest of a
// SerializableTypeSystem using the requested mode.
func DigestWithMode(
	typeSystem *typesystem.SerializableTypeSystem,
	mode Mode,
) ([]byte, error) {
	return serializableTypeSystemNode{value: typeSystem}.DigestWithMode(mode)
}

// TypeIDDigest computes the canonical 32-byte SHA-256 digest of one TypeId.
func TypeIDDigest(typeID *typeid.TypeId) ([]byte, error) {
	return digest(typeIDNode{value: typeID})
}

func validateMode(mode Mode) error {
	switch mode {
	case ModeFull, ModeStructural:
		return nil
	default:
		return fmt.Errorf("unknown digest mode: %d", mode)
	}
}

// typeSystemDigester is implemented by nodes that can produce a TypeSystem digest.
type typeSystemDigester interface {
	Digest() ([]byte, error)
	DigestWithMode(Mode) ([]byte, error)
	digestWriter
}

type digestWriter interface {
	hashInto(*hasher) error
}

type keyDigestWriter interface {
	digestWriter
	hashKeyInto(*hasher) error
}

func digest(value typeSystemDigester) ([]byte, error) {
	return digestWithMode(value, ModeFull)
}

func digestWithMode(value digestWriter, mode Mode) ([]byte, error) {
	if err := validateMode(mode); err != nil {
		return nil, err
	}

	h := newHasher(mode)
	if err := h.hashInto(value); err != nil {
		return nil, err
	}
	return h.finalize(), nil
}

type hasher struct {
	h    hash.Hash
	mode Mode
}

func newHasher(mode Mode) *hasher {
	return &hasher{
		h:    sha256.New(),
		mode: mode,
	}
}

func (h *hasher) includeAnnotationsAndDefaults() bool {
	return h.mode == ModeFull
}

func (h *hasher) finalize() []byte {
	return h.h.Sum(nil)
}

func (h *hasher) write(value []byte) {
	_, _ = h.h.Write(value)
}

func (h *hasher) writeByte(value byte) {
	h.write([]byte{value})
}

func (h *hasher) writeBool(value bool) {
	if value {
		h.writeU8(1)
		return
	}
	h.writeU8(0)
}

func (h *hasher) writeU8(value byte) {
	h.writeByte(value)
}

func (h *hasher) writeI8(value int8) {
	h.writeByte(byte(value))
}

func (h *hasher) writeI16(value int16) {
	var buf [2]byte
	binary.LittleEndian.PutUint16(buf[:], uint16(value))
	h.write(buf[:])
}

func (h *hasher) writeI32(value int32) {
	var buf [4]byte
	binary.LittleEndian.PutUint32(buf[:], uint32(value))
	h.write(buf[:])
}

func (h *hasher) writeI64(value int64) {
	var buf [8]byte
	binary.LittleEndian.PutUint64(buf[:], uint64(value))
	h.write(buf[:])
}

func (h *hasher) writeU32(value uint32) {
	var buf [4]byte
	binary.LittleEndian.PutUint32(buf[:], value)
	h.write(buf[:])
}

func (h *hasher) writeF32(value float32) {
	var buf [4]byte
	binary.LittleEndian.PutUint32(buf[:], math.Float32bits(value))
	h.write(buf[:])
}

func (h *hasher) writeF64(value float64) {
	var buf [8]byte
	binary.LittleEndian.PutUint64(buf[:], math.Float64bits(value))
	h.write(buf[:])
}

func (h *hasher) writeString(value string) {
	h.writeBytes([]byte(value))
}

func (h *hasher) writeBytes(value []byte) {
	h.writeU32(uint32(len(value)))
	h.write(value)
}

func (h *hasher) hashInto(value digestWriter) error {
	return value.hashInto(h)
}

func (h *hasher) hashAll(values ...digestWriter) error {
	for _, value := range values {
		if err := h.hashInto(value); err != nil {
			return err
		}
	}
	return nil
}

func (h *hasher) hashUnorderedByDigest(values []digestWriter) error {
	digests := make([][]byte, 0, len(values))
	for _, value := range values {
		sub := newHasher(h.mode)
		if err := sub.hashInto(value); err != nil {
			return err
		}
		digests = append(digests, sub.finalize())
	}
	h.writeU32(uint32(len(digests)))
	slices.SortFunc(digests, bytes.Compare)
	for _, digest := range digests {
		h.write(digest)
	}
	return nil
}

func (h *hasher) hashMapByKeyDigest(values []keyDigestWriter) error {
	type digestEntry struct {
		keyDigest []byte
		value     keyDigestWriter
	}

	entries := make([]digestEntry, 0, len(values))
	for _, value := range values {
		sub := newHasher(h.mode)
		if err := value.hashKeyInto(sub); err != nil {
			return err
		}
		entries = append(entries, digestEntry{
			keyDigest: sub.finalize(),
			value:     value,
		})
	}
	h.writeU32(uint32(len(entries)))
	slices.SortStableFunc(entries, func(a, b digestEntry) int {
		return bytes.Compare(a.keyDigest, b.keyDigest)
	})
	for _, entry := range entries {
		if err := h.hashInto(entry.value); err != nil {
			return err
		}
	}
	return nil
}

func (h *hasher) hashAnnotations(
	annotations map[typeid.Uri]*record.SerializableRecord,
) error {
	if !h.includeAnnotationsAndDefaults() {
		return nil
	}
	values := make([]digestWriter, 0, len(annotations))
	for uri, value := range annotations {
		values = append(values, annotationNode{
			uri:   uri,
			value: value,
		})
	}
	return h.hashUnorderedByDigest(values)
}

type serializableTypeSystemNode struct {
	value *typesystem.SerializableTypeSystem
}

func (n serializableTypeSystemNode) Digest() ([]byte, error) {
	return digest(n)
}

func (n serializableTypeSystemNode) DigestWithMode(mode Mode) ([]byte, error) {
	if n.value == nil {
		return nil, fmt.Errorf("cannot digest nil SerializableTypeSystem")
	}
	return digestWithMode(n, mode)
}

func (n serializableTypeSystemNode) hashInto(h *hasher) error {
	if n.value == nil {
		return fmt.Errorf("cannot digest nil SerializableTypeSystem")
	}
	h.writeU8(digestVersion)

	uris := make([]typeid.Uri, 0, len(n.value.GetTypes()))
	for uri := range n.value.GetTypes() {
		uris = append(uris, uri)
	}
	slices.SortFunc(uris, func(a, b typeid.Uri) int {
		return cmp.Compare(string(a), string(b))
	})

	for _, uri := range uris {
		entry := n.value.GetTypes()[uri]
		if entry == nil || entry.GetDefinition() == nil {
			return fmt.Errorf("type %q has no definition", uri)
		}
		h.writeString(string(uri))
		if err := h.hashInto(typeDefinitionNode{value: entry.GetDefinition()}); err != nil {
			return fmt.Errorf("type %q: %w", uri, err)
		}
	}

	return nil
}

type typeIDNode struct {
	value *typeid.TypeId
}

func (n typeIDNode) Digest() ([]byte, error) {
	return digest(n)
}

func (n typeIDNode) DigestWithMode(mode Mode) ([]byte, error) {
	return digestWithMode(n, mode)
}

func (n typeIDNode) hashInto(h *hasher) error {
	typeID := n.value
	if typeID == nil {
		h.writeU8(0)
		return nil
	}
	if count := typeID.CountSetFields(); count == 0 {
		h.writeU8(0)
		return nil
	} else if count != 1 {
		return fmt.Errorf("TypeId union has %d fields set", count)
	}

	switch {
	case typeID.BoolType != nil:
		h.writeI32(1)
	case typeID.ByteType != nil:
		h.writeI32(2)
	case typeID.I16Type != nil:
		h.writeI32(3)
	case typeID.I32Type != nil:
		h.writeI32(4)
	case typeID.I64Type != nil:
		h.writeI32(5)
	case typeID.FloatType != nil:
		h.writeI32(6)
	case typeID.DoubleType != nil:
		h.writeI32(7)
	case typeID.StringType != nil:
		h.writeI32(8)
	case typeID.BinaryType != nil:
		h.writeI32(9)
	case typeID.AnyType != nil:
		h.writeI32(10)
	case typeID.UserDefinedType != nil:
		h.writeI32(11)
		h.writeString(string(*typeID.UserDefinedType))
	case typeID.ListType != nil:
		h.writeI32(12)
		return h.hashInto(typeIDNode{value: typeID.ListType.GetElementType()})
	case typeID.SetType != nil:
		h.writeI32(13)
		return h.hashInto(typeIDNode{value: typeID.SetType.GetElementType()})
	case typeID.MapType != nil:
		h.writeI32(14)
		if err := h.hashInto(typeIDNode{value: typeID.MapType.GetKeyType()}); err != nil {
			return err
		}
		return h.hashInto(typeIDNode{value: typeID.MapType.GetValueType()})
	}
	return nil
}

type recordNode struct {
	value *record.SerializableRecord
}

func (n recordNode) hashInto(h *hasher) error {
	value := n.value
	if value == nil {
		return fmt.Errorf("cannot digest nil SerializableRecord")
	}
	if count := value.CountSetFields(); count != 1 {
		return fmt.Errorf("SerializableRecord union has %d fields set", count)
	}

	switch {
	case value.BoolDatum != nil:
		h.writeI32(1)
		h.writeBool(*value.BoolDatum)
	case value.Int8Datum != nil:
		h.writeI32(2)
		h.writeI8(*value.Int8Datum)
	case value.Int16Datum != nil:
		h.writeI32(3)
		h.writeI16(*value.Int16Datum)
	case value.Int32Datum != nil:
		h.writeI32(5)
		h.writeI32(*value.Int32Datum)
	case value.Int64Datum != nil:
		h.writeI32(6)
		h.writeI64(*value.Int64Datum)
	case value.Float32Datum != nil:
		h.writeI32(7)
		h.writeF32(*value.Float32Datum)
	case value.Float64Datum != nil:
		h.writeI32(8)
		h.writeF64(*value.Float64Datum)
	case value.TextDatum != nil:
		h.writeI32(9)
		h.writeString(*value.TextDatum)
	case value.ByteArrayDatum != nil:
		h.writeI32(10)
		h.writeBytes(value.ByteArrayDatum)
	case value.FieldSetDatum != nil:
		h.writeI32(11)
		keys := make([]int16, 0, len(value.FieldSetDatum))
		for fieldID := range value.FieldSetDatum {
			keys = append(keys, int16(fieldID))
		}
		slices.Sort(keys)
		for _, fieldID := range keys {
			h.writeI16(fieldID)
			if err := h.hashInto(recordNode{value: value.FieldSetDatum[typesystem.FieldId(fieldID)]}); err != nil {
				return err
			}
		}
	case value.ListDatum != nil:
		h.writeI32(12)
		for _, element := range value.ListDatum {
			if err := h.hashInto(recordNode{value: element}); err != nil {
				return err
			}
		}
	case value.SetDatum != nil:
		h.writeI32(13)
		values := make([]digestWriter, 0, len(value.SetDatum))
		for _, element := range value.SetDatum {
			values = append(values, recordNode{value: element})
		}
		if err := h.hashUnorderedByDigest(values); err != nil {
			return err
		}
	case value.MapDatum != nil:
		h.writeI32(14)
		values := make([]keyDigestWriter, 0, len(value.MapDatum))
		for _, entry := range value.MapDatum {
			values = append(values, recordMapEntryNode{value: entry})
		}
		return h.hashMapByKeyDigest(values)
	}
	return nil
}

type recordMapEntryNode struct {
	value *record.SerializableRecordMapEntry
}

func (n recordMapEntryNode) hashKeyInto(h *hasher) error {
	if n.value == nil {
		return fmt.Errorf("cannot digest nil SerializableRecordMapEntry")
	}
	return h.hashInto(recordNode{value: n.value.GetKey()})
}

func (n recordMapEntryNode) hashInto(h *hasher) error {
	if n.value == nil {
		return fmt.Errorf("cannot digest nil SerializableRecordMapEntry")
	}
	return h.hashAll(
		recordNode{value: n.value.GetKey()},
		recordNode{value: n.value.GetValue()},
	)
}

type annotationNode struct {
	uri   typeid.Uri
	value *record.SerializableRecord
}

func (n annotationNode) hashInto(h *hasher) error {
	h.writeString(string(n.uri))
	return h.hashInto(recordNode{value: n.value})
}

type typeDefinitionNode struct {
	value *typesystem.SerializableTypeDefinition
}

func (n typeDefinitionNode) hashInto(h *hasher) error {
	definition := n.value
	if definition == nil {
		return fmt.Errorf("SerializableTypeDefinition union has 0 fields set")
	}
	if count := definition.CountSetFields(); count != 1 {
		return fmt.Errorf("SerializableTypeDefinition union has %d fields set", count)
	}

	switch {
	case definition.StructDef != nil:
		h.writeI32(1)
		return h.hashInto(structDefinitionNode{value: definition.StructDef})
	case definition.UnionDef != nil:
		h.writeI32(2)
		return h.hashInto(unionDefinitionNode{value: definition.UnionDef})
	case definition.EnumDef != nil:
		h.writeI32(3)
		return h.hashInto(enumDefinitionNode{value: definition.EnumDef})
	case definition.OpaqueAliasDef != nil:
		h.writeI32(4)
		return h.hashInto(opaqueAliasDefinitionNode{value: definition.OpaqueAliasDef})
	default:
		return nil
	}
}

type structDefinitionNode struct {
	value *typesystem.SerializableStructDefinition
}

func (n structDefinitionNode) hashInto(h *hasher) error {
	if n.value == nil {
		return fmt.Errorf("cannot digest nil SerializableStructDefinition")
	}
	fields := slices.Clone(n.value.GetFields())
	slices.SortFunc(fields, func(a, b *typesystem.SerializableFieldDefinition) int {
		return cmp.Compare(fieldSortID(a), fieldSortID(b))
	})
	for _, field := range fields {
		if err := h.hashInto(fieldDefinitionNode{value: field}); err != nil {
			return err
		}
	}
	h.writeBool(n.value.GetIsSealed())
	return h.hashAnnotations(n.value.GetAnnotations())
}

type unionDefinitionNode struct {
	value *typesystem.SerializableUnionDefinition
}

func (n unionDefinitionNode) hashInto(h *hasher) error {
	if n.value == nil {
		return fmt.Errorf("cannot digest nil SerializableUnionDefinition")
	}
	fields := slices.Clone(n.value.GetFields())
	slices.SortFunc(fields, func(a, b *typesystem.SerializableFieldDefinition) int {
		return cmp.Compare(fieldSortID(a), fieldSortID(b))
	})
	for _, field := range fields {
		if err := h.hashInto(fieldDefinitionNode{value: field}); err != nil {
			return err
		}
	}
	h.writeBool(n.value.GetIsSealed())
	return h.hashAnnotations(n.value.GetAnnotations())
}

type fieldDefinitionNode struct {
	value *typesystem.SerializableFieldDefinition
}

func (n fieldDefinitionNode) hashInto(h *hasher) error {
	field := n.value
	if field == nil {
		return fmt.Errorf("cannot digest nil SerializableFieldDefinition")
	}
	if field.GetIdentity() == nil {
		return fmt.Errorf("field has no identity")
	}
	h.writeI16(field.GetIdentity().GetId())
	h.writeString(field.GetIdentity().GetName())
	h.writeI32(int32(field.GetPresence()))
	if err := h.hashInto(typeIDNode{value: field.GetType()}); err != nil {
		return err
	}
	if h.includeAnnotationsAndDefaults() && field.GetCustomDefaultPartialRecord() != nil {
		if err := h.hashInto(recordNode{value: field.GetCustomDefaultPartialRecord()}); err != nil {
			return err
		}
	}
	return h.hashAnnotations(field.GetAnnotations())
}

func fieldSortID(field *typesystem.SerializableFieldDefinition) int16 {
	if field == nil || field.GetIdentity() == nil {
		return 0
	}
	return int16(field.GetIdentity().GetId())
}

type enumDefinitionNode struct {
	value *typesystem.SerializableEnumDefinition
}

func (n enumDefinitionNode) hashInto(h *hasher) error {
	if n.value == nil {
		return fmt.Errorf("cannot digest nil SerializableEnumDefinition")
	}
	values := slices.Clone(n.value.GetValues())
	slices.SortFunc(values, func(a, b *typesystem.SerializableEnumValueDefinition) int {
		return cmp.Compare(enumSortDatum(a), enumSortDatum(b))
	})
	for _, value := range values {
		if err := h.hashInto(enumValueNode{value: value}); err != nil {
			return err
		}
	}
	return h.hashAnnotations(n.value.GetAnnotations())
}

type enumValueNode struct {
	value *typesystem.SerializableEnumValueDefinition
}

func (n enumValueNode) hashInto(h *hasher) error {
	if n.value == nil {
		return fmt.Errorf("cannot digest nil SerializableEnumValueDefinition")
	}
	h.writeString(n.value.GetName())
	h.writeI32(n.value.GetDatum())
	return h.hashAnnotations(n.value.GetAnnotations())
}

func enumSortDatum(value *typesystem.SerializableEnumValueDefinition) int32 {
	if value == nil {
		return 0
	}
	return value.GetDatum()
}

type opaqueAliasDefinitionNode struct {
	value *typesystem.SerializableOpaqueAliasDefinition
}

func (n opaqueAliasDefinitionNode) hashInto(h *hasher) error {
	if n.value == nil {
		return fmt.Errorf("cannot digest nil SerializableOpaqueAliasDefinition")
	}
	return h.hashAll(
		typeIDNode{value: n.value.GetTargetType()},
		annotationSetNode{values: n.value.GetAnnotations()},
	)
}

type annotationSetNode struct {
	values map[typeid.Uri]*record.SerializableRecord
}

func (n annotationSetNode) hashInto(h *hasher) error {
	return h.hashAnnotations(n.values)
}
