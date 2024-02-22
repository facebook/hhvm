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

package thrift

import (
	"log"
)

type DebugProtocol struct {
	Delegate  Protocol
	LogPrefix string
}

type DebugProtocolFactory struct {
	Underlying ProtocolFactory
	LogPrefix  string
}

func NewDebugProtocolFactory(underlying ProtocolFactory, logPrefix string) *DebugProtocolFactory {
	return &DebugProtocolFactory{
		Underlying: underlying,
		LogPrefix:  logPrefix,
	}
}

func (t *DebugProtocolFactory) GetProtocol(trans Transport) Protocol {
	return &DebugProtocol{
		Delegate:  t.Underlying.GetProtocol(trans),
		LogPrefix: t.LogPrefix,
	}
}

func (tdp *DebugProtocol) WriteMessageBegin(name string, typeId MessageType, seqid int32) error {
	err := tdp.Delegate.WriteMessageBegin(name, typeId, seqid)
	log.Printf("%sWriteMessageBegin(name=%#v, typeId=%#v, seqid=%#v) => %#v", tdp.LogPrefix, name, typeId, seqid, err)
	return err
}
func (tdp *DebugProtocol) WriteMessageEnd() error {
	err := tdp.Delegate.WriteMessageEnd()
	log.Printf("%sWriteMessageEnd() => %#v", tdp.LogPrefix, err)
	return err
}
func (tdp *DebugProtocol) WriteStructBegin(name string) error {
	err := tdp.Delegate.WriteStructBegin(name)
	log.Printf("%sWriteStructBegin(name=%#v) => %#v", tdp.LogPrefix, name, err)
	return err
}
func (tdp *DebugProtocol) WriteStructEnd() error {
	err := tdp.Delegate.WriteStructEnd()
	log.Printf("%sWriteStructEnd() => %#v", tdp.LogPrefix, err)
	return err
}
func (tdp *DebugProtocol) WriteFieldBegin(name string, typeId Type, id int16) error {
	err := tdp.Delegate.WriteFieldBegin(name, typeId, id)
	log.Printf("%sWriteFieldBegin(name=%#v, typeId=%#v, id%#v) => %#v", tdp.LogPrefix, name, typeId, id, err)
	return err
}
func (tdp *DebugProtocol) WriteFieldEnd() error {
	err := tdp.Delegate.WriteFieldEnd()
	log.Printf("%sWriteFieldEnd() => %#v", tdp.LogPrefix, err)
	return err
}
func (tdp *DebugProtocol) WriteFieldStop() error {
	err := tdp.Delegate.WriteFieldStop()
	log.Printf("%sWriteFieldStop() => %#v", tdp.LogPrefix, err)
	return err
}
func (tdp *DebugProtocol) WriteMapBegin(keyType Type, valueType Type, size int) error {
	err := tdp.Delegate.WriteMapBegin(keyType, valueType, size)
	log.Printf("%sWriteMapBegin(keyType=%#v, valueType=%#v, size=%#v) => %#v", tdp.LogPrefix, keyType, valueType, size, err)
	return err
}
func (tdp *DebugProtocol) WriteMapEnd() error {
	err := tdp.Delegate.WriteMapEnd()
	log.Printf("%sWriteMapEnd() => %#v", tdp.LogPrefix, err)
	return err
}
func (tdp *DebugProtocol) WriteListBegin(elemType Type, size int) error {
	err := tdp.Delegate.WriteListBegin(elemType, size)
	log.Printf("%sWriteListBegin(elemType=%#v, size=%#v) => %#v", tdp.LogPrefix, elemType, size, err)
	return err
}
func (tdp *DebugProtocol) WriteListEnd() error {
	err := tdp.Delegate.WriteListEnd()
	log.Printf("%sWriteListEnd() => %#v", tdp.LogPrefix, err)
	return err
}
func (tdp *DebugProtocol) WriteSetBegin(elemType Type, size int) error {
	err := tdp.Delegate.WriteSetBegin(elemType, size)
	log.Printf("%sWriteSetBegin(elemType=%#v, size=%#v) => %#v", tdp.LogPrefix, elemType, size, err)
	return err
}
func (tdp *DebugProtocol) WriteSetEnd() error {
	err := tdp.Delegate.WriteSetEnd()
	log.Printf("%sWriteSetEnd() => %#v", tdp.LogPrefix, err)
	return err
}
func (tdp *DebugProtocol) WriteBool(value bool) error {
	err := tdp.Delegate.WriteBool(value)
	log.Printf("%sWriteBool(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *DebugProtocol) WriteByte(value byte) error {
	err := tdp.Delegate.WriteByte(value)
	log.Printf("%sWriteByte(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *DebugProtocol) WriteI16(value int16) error {
	err := tdp.Delegate.WriteI16(value)
	log.Printf("%sWriteI16(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *DebugProtocol) WriteI32(value int32) error {
	err := tdp.Delegate.WriteI32(value)
	log.Printf("%sWriteI32(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *DebugProtocol) WriteI64(value int64) error {
	err := tdp.Delegate.WriteI64(value)
	log.Printf("%sWriteI64(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *DebugProtocol) WriteDouble(value float64) error {
	err := tdp.Delegate.WriteDouble(value)
	log.Printf("%sWriteDouble(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *DebugProtocol) WriteFloat(value float32) error {
	err := tdp.Delegate.WriteFloat(value)
	log.Printf("%sWriteFloat(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *DebugProtocol) WriteString(value string) error {
	err := tdp.Delegate.WriteString(value)
	log.Printf("%sWriteString(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *DebugProtocol) WriteBinary(value []byte) error {
	err := tdp.Delegate.WriteBinary(value)
	log.Printf("%sWriteBinary(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}

func (tdp *DebugProtocol) ReadMessageBegin() (name string, typeId MessageType, seqid int32, err error) {
	name, typeId, seqid, err = tdp.Delegate.ReadMessageBegin()
	log.Printf("%sReadMessageBegin() (name=%#v, typeId=%#v, seqid=%#v, err=%#v)", tdp.LogPrefix, name, typeId, seqid, err)
	return
}
func (tdp *DebugProtocol) ReadMessageEnd() (err error) {
	err = tdp.Delegate.ReadMessageEnd()
	log.Printf("%sReadMessageEnd() err=%#v", tdp.LogPrefix, err)
	return
}
func (tdp *DebugProtocol) ReadStructBegin() (name string, err error) {
	name, err = tdp.Delegate.ReadStructBegin()
	log.Printf("%sReadStructBegin() (name%#v, err=%#v)", tdp.LogPrefix, name, err)
	return
}
func (tdp *DebugProtocol) ReadStructEnd() (err error) {
	err = tdp.Delegate.ReadStructEnd()
	log.Printf("%sReadStructEnd() err=%#v", tdp.LogPrefix, err)
	return
}
func (tdp *DebugProtocol) ReadFieldBegin() (name string, typeId Type, id int16, err error) {
	name, typeId, id, err = tdp.Delegate.ReadFieldBegin()
	log.Printf("%sReadFieldBegin() (name=%#v, typeId=%#v, id=%#v, err=%#v)", tdp.LogPrefix, name, typeId, id, err)
	return
}
func (tdp *DebugProtocol) ReadFieldEnd() (err error) {
	err = tdp.Delegate.ReadFieldEnd()
	log.Printf("%sReadFieldEnd() err=%#v", tdp.LogPrefix, err)
	return
}
func (tdp *DebugProtocol) ReadMapBegin() (keyType Type, valueType Type, size int, err error) {
	keyType, valueType, size, err = tdp.Delegate.ReadMapBegin()
	log.Printf("%sReadMapBegin() (keyType=%#v, valueType=%#v, size=%#v, err=%#v)", tdp.LogPrefix, keyType, valueType, size, err)
	return
}
func (tdp *DebugProtocol) ReadMapEnd() (err error) {
	err = tdp.Delegate.ReadMapEnd()
	log.Printf("%sReadMapEnd() err=%#v", tdp.LogPrefix, err)
	return
}
func (tdp *DebugProtocol) ReadListBegin() (elemType Type, size int, err error) {
	elemType, size, err = tdp.Delegate.ReadListBegin()
	log.Printf("%sReadListBegin() (elemType=%#v, size=%#v, err=%#v)", tdp.LogPrefix, elemType, size, err)
	return
}
func (tdp *DebugProtocol) ReadListEnd() (err error) {
	err = tdp.Delegate.ReadListEnd()
	log.Printf("%sReadListEnd() err=%#v", tdp.LogPrefix, err)
	return
}
func (tdp *DebugProtocol) ReadSetBegin() (elemType Type, size int, err error) {
	elemType, size, err = tdp.Delegate.ReadSetBegin()
	log.Printf("%sReadSetBegin() (elemType=%#v, size=%#v, err=%#v)", tdp.LogPrefix, elemType, size, err)
	return
}
func (tdp *DebugProtocol) ReadSetEnd() (err error) {
	err = tdp.Delegate.ReadSetEnd()
	log.Printf("%sReadSetEnd() err=%#v", tdp.LogPrefix, err)
	return
}
func (tdp *DebugProtocol) ReadBool() (value bool, err error) {
	value, err = tdp.Delegate.ReadBool()
	log.Printf("%sReadBool() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return
}
func (tdp *DebugProtocol) ReadByte() (value byte, err error) {
	value, err = tdp.Delegate.ReadByte()
	log.Printf("%sReadByte() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return
}
func (tdp *DebugProtocol) ReadI16() (value int16, err error) {
	value, err = tdp.Delegate.ReadI16()
	log.Printf("%sReadI16() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return
}
func (tdp *DebugProtocol) ReadI32() (value int32, err error) {
	value, err = tdp.Delegate.ReadI32()
	log.Printf("%sReadI32() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return
}
func (tdp *DebugProtocol) ReadI64() (value int64, err error) {
	value, err = tdp.Delegate.ReadI64()
	log.Printf("%sReadI64() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return
}
func (tdp *DebugProtocol) ReadDouble() (value float64, err error) {
	value, err = tdp.Delegate.ReadDouble()
	log.Printf("%sReadDouble() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return
}
func (tdp *DebugProtocol) ReadFloat() (value float32, err error) {
	value, err = tdp.Delegate.ReadFloat()
	log.Printf("%sReadFloat() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return
}
func (tdp *DebugProtocol) ReadString() (value string, err error) {
	value, err = tdp.Delegate.ReadString()
	log.Printf("%sReadString() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return
}
func (tdp *DebugProtocol) ReadBinary() (value []byte, err error) {
	value, err = tdp.Delegate.ReadBinary()
	log.Printf("%sReadBinary() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return
}
func (tdp *DebugProtocol) Skip(fieldType Type) (err error) {
	err = tdp.Delegate.Skip(fieldType)
	log.Printf("%sSkip(fieldType=%#v) (err=%#v)", tdp.LogPrefix, fieldType, err)
	return
}
func (tdp *DebugProtocol) Flush() (err error) {
	err = tdp.Delegate.Flush()
	log.Printf("%sFlush() (err=%#v)", tdp.LogPrefix, err)
	return
}

// Deprecated: Transport() is a deprecated method.
func (tdp *DebugProtocol) Transport() Transport {
	return tdp.Delegate.Transport()
}

func (tdp *DebugProtocol) Close() error {
	return tdp.Delegate.Close()
}
