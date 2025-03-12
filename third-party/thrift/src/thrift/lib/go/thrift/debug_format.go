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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

type debugFormat struct {
	Delegate  types.Format
	LogPrefix string
}

var _ types.Format = (*debugFormat)(nil)

func (tdp *debugFormat) WriteMessageBegin(name string, typeId types.MessageType, seqid int32) error {
	err := tdp.Delegate.WriteMessageBegin(name, typeId, seqid)
	log.Printf("%sWriteMessageBegin(name=%#v, typeId=%#v, seqid=%#v) => %#v", tdp.LogPrefix, name, typeId, seqid, err)
	return err
}
func (tdp *debugFormat) WriteMessageEnd() error {
	err := tdp.Delegate.WriteMessageEnd()
	log.Printf("%sWriteMessageEnd() => %#v", tdp.LogPrefix, err)
	return err
}
func (tdp *debugFormat) WriteStructBegin(name string) error {
	err := tdp.Delegate.WriteStructBegin(name)
	log.Printf("%sWriteStructBegin(name=%#v) => %#v", tdp.LogPrefix, name, err)
	return err
}
func (tdp *debugFormat) WriteStructEnd() error {
	err := tdp.Delegate.WriteStructEnd()
	log.Printf("%sWriteStructEnd() => %#v", tdp.LogPrefix, err)
	return err
}
func (tdp *debugFormat) WriteFieldBegin(name string, typeId types.Type, id int16) error {
	err := tdp.Delegate.WriteFieldBegin(name, typeId, id)
	log.Printf("%sWriteFieldBegin(name=%#v, typeId=%#v, id%#v) => %#v", tdp.LogPrefix, name, typeId, id, err)
	return err
}
func (tdp *debugFormat) WriteFieldEnd() error {
	err := tdp.Delegate.WriteFieldEnd()
	log.Printf("%sWriteFieldEnd() => %#v", tdp.LogPrefix, err)
	return err
}
func (tdp *debugFormat) WriteFieldStop() error {
	err := tdp.Delegate.WriteFieldStop()
	log.Printf("%sWriteFieldStop() => %#v", tdp.LogPrefix, err)
	return err
}
func (tdp *debugFormat) WriteMapBegin(keyType types.Type, valueType types.Type, size int) error {
	err := tdp.Delegate.WriteMapBegin(keyType, valueType, size)
	log.Printf("%sWriteMapBegin(keyType=%#v, valueType=%#v, size=%#v) => %#v", tdp.LogPrefix, keyType, valueType, size, err)
	return err
}
func (tdp *debugFormat) WriteMapEnd() error {
	err := tdp.Delegate.WriteMapEnd()
	log.Printf("%sWriteMapEnd() => %#v", tdp.LogPrefix, err)
	return err
}
func (tdp *debugFormat) WriteListBegin(elemType types.Type, size int) error {
	err := tdp.Delegate.WriteListBegin(elemType, size)
	log.Printf("%sWriteListBegin(elemType=%#v, size=%#v) => %#v", tdp.LogPrefix, elemType, size, err)
	return err
}
func (tdp *debugFormat) WriteListEnd() error {
	err := tdp.Delegate.WriteListEnd()
	log.Printf("%sWriteListEnd() => %#v", tdp.LogPrefix, err)
	return err
}
func (tdp *debugFormat) WriteSetBegin(elemType types.Type, size int) error {
	err := tdp.Delegate.WriteSetBegin(elemType, size)
	log.Printf("%sWriteSetBegin(elemType=%#v, size=%#v) => %#v", tdp.LogPrefix, elemType, size, err)
	return err
}
func (tdp *debugFormat) WriteSetEnd() error {
	err := tdp.Delegate.WriteSetEnd()
	log.Printf("%sWriteSetEnd() => %#v", tdp.LogPrefix, err)
	return err
}
func (tdp *debugFormat) WriteBool(value bool) error {
	err := tdp.Delegate.WriteBool(value)
	log.Printf("%sWriteBool(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *debugFormat) WriteByte(value byte) error {
	err := tdp.Delegate.WriteByte(value)
	log.Printf("%sWriteByte(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *debugFormat) WriteI16(value int16) error {
	err := tdp.Delegate.WriteI16(value)
	log.Printf("%sWriteI16(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *debugFormat) WriteI32(value int32) error {
	err := tdp.Delegate.WriteI32(value)
	log.Printf("%sWriteI32(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *debugFormat) WriteI64(value int64) error {
	err := tdp.Delegate.WriteI64(value)
	log.Printf("%sWriteI64(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *debugFormat) WriteDouble(value float64) error {
	err := tdp.Delegate.WriteDouble(value)
	log.Printf("%sWriteDouble(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *debugFormat) WriteFloat(value float32) error {
	err := tdp.Delegate.WriteFloat(value)
	log.Printf("%sWriteFloat(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *debugFormat) WriteString(value string) error {
	err := tdp.Delegate.WriteString(value)
	log.Printf("%sWriteString(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}
func (tdp *debugFormat) WriteBinary(value []byte) error {
	err := tdp.Delegate.WriteBinary(value)
	log.Printf("%sWriteBinary(value=%#v) => %#v", tdp.LogPrefix, value, err)
	return err
}

func (tdp *debugFormat) ReadMessageBegin() (name string, typeId types.MessageType, seqid int32, err error) {
	name, typeId, seqid, err = tdp.Delegate.ReadMessageBegin()
	log.Printf("%sReadMessageBegin() (name=%#v, typeId=%#v, seqid=%#v, err=%#v)", tdp.LogPrefix, name, typeId, seqid, err)
	return
}
func (tdp *debugFormat) ReadMessageEnd() error {
	err := tdp.Delegate.ReadMessageEnd()
	log.Printf("%sReadMessageEnd() err=%#v", tdp.LogPrefix, err)
	return err
}
func (tdp *debugFormat) ReadStructBegin() (string, error) {
	name, err := tdp.Delegate.ReadStructBegin()
	log.Printf("%sReadStructBegin() (name%#v, err=%#v)", tdp.LogPrefix, name, err)
	return name, err
}
func (tdp *debugFormat) ReadStructEnd() error {
	err := tdp.Delegate.ReadStructEnd()
	log.Printf("%sReadStructEnd() err=%#v", tdp.LogPrefix, err)
	return err
}
func (tdp *debugFormat) ReadFieldBegin() (name string, typeId types.Type, id int16, err error) {
	name, typeId, id, err = tdp.Delegate.ReadFieldBegin()
	log.Printf("%sReadFieldBegin() (name=%#v, typeId=%#v, id=%#v, err=%#v)", tdp.LogPrefix, name, typeId, id, err)
	return
}
func (tdp *debugFormat) ReadFieldEnd() error {
	err := tdp.Delegate.ReadFieldEnd()
	log.Printf("%sReadFieldEnd() err=%#v", tdp.LogPrefix, err)
	return err
}
func (tdp *debugFormat) ReadMapBegin() (types.Type /* kType */, types.Type /* vType */, int /* size */, error) {
	kType, vType, size, err := tdp.Delegate.ReadMapBegin()
	log.Printf("%sReadMapBegin() (keyType=%#v, valueType=%#v, size=%#v, err=%#v)", tdp.LogPrefix, kType, vType, size, err)
	return kType, vType, size, err
}
func (tdp *debugFormat) ReadMapEnd() error {
	err := tdp.Delegate.ReadMapEnd()
	log.Printf("%sReadMapEnd() err=%#v", tdp.LogPrefix, err)
	return err
}
func (tdp *debugFormat) ReadListBegin() (types.Type /* elemType */, int /* size */, error) {
	elemType, size, err := tdp.Delegate.ReadListBegin()
	log.Printf("%sReadListBegin() (elemType=%#v, size=%#v, err=%#v)", tdp.LogPrefix, elemType, size, err)
	return elemType, size, err
}
func (tdp *debugFormat) ReadListEnd() error {
	err := tdp.Delegate.ReadListEnd()
	log.Printf("%sReadListEnd() err=%#v", tdp.LogPrefix, err)
	return err
}
func (tdp *debugFormat) ReadSetBegin() (types.Type /* elemType */, int /* size */, error) {
	elemType, size, err := tdp.Delegate.ReadSetBegin()
	log.Printf("%sReadSetBegin() (elemType=%#v, size=%#v, err=%#v)", tdp.LogPrefix, elemType, size, err)
	return elemType, size, err
}
func (tdp *debugFormat) ReadSetEnd() error {
	err := tdp.Delegate.ReadSetEnd()
	log.Printf("%sReadSetEnd() err=%#v", tdp.LogPrefix, err)
	return err
}
func (tdp *debugFormat) ReadBool() (bool, error) {
	value, err := tdp.Delegate.ReadBool()
	log.Printf("%sReadBool() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return value, err
}
func (tdp *debugFormat) ReadByte() (byte, error) {
	value, err := tdp.Delegate.ReadByte()
	log.Printf("%sReadByte() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return value, err
}
func (tdp *debugFormat) ReadI16() (int16, error) {
	value, err := tdp.Delegate.ReadI16()
	log.Printf("%sReadI16() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return value, err
}
func (tdp *debugFormat) ReadI32() (int32, error) {
	value, err := tdp.Delegate.ReadI32()
	log.Printf("%sReadI32() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return value, err
}
func (tdp *debugFormat) ReadI64() (int64, error) {
	value, err := tdp.Delegate.ReadI64()
	log.Printf("%sReadI64() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return value, err
}
func (tdp *debugFormat) ReadDouble() (float64, error) {
	value, err := tdp.Delegate.ReadDouble()
	log.Printf("%sReadDouble() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return value, err
}
func (tdp *debugFormat) ReadFloat() (float32, error) {
	value, err := tdp.Delegate.ReadFloat()
	log.Printf("%sReadFloat() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return value, err
}
func (tdp *debugFormat) ReadString() (string, error) {
	value, err := tdp.Delegate.ReadString()
	log.Printf("%sReadString() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return value, err
}
func (tdp *debugFormat) ReadBinary() ([]byte, error) {
	value, err := tdp.Delegate.ReadBinary()
	log.Printf("%sReadBinary() (value=%#v, err=%#v)", tdp.LogPrefix, value, err)
	return value, err
}
func (tdp *debugFormat) Skip(fieldType types.Type) error {
	err := tdp.Delegate.Skip(fieldType)
	log.Printf("%sSkip(fieldType=%#v) (err=%#v)", tdp.LogPrefix, fieldType, err)
	return err
}
func (tdp *debugFormat) Flush() error {
	err := tdp.Delegate.Flush()
	log.Printf("%sFlush() (err=%#v)", tdp.LogPrefix, err)
	return err
}
