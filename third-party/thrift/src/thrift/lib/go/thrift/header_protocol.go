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
	"fmt"
	"net"
	"time"
)

type headerProtocol struct {
	Format
	trans *headerTransport

	protoID ProtocolID
}

func NewHeaderProtocol(conn net.Conn) (Protocol, error) {
	p := &headerProtocol{
		protoID: ProtocolIDCompact,
	}
	p.trans = newHeaderTransport(conn)
	if err := p.resetProtocol(); err != nil {
		return nil, err
	}
	return p, nil
}

func (p *headerProtocol) SetTimeout(timeout time.Duration) {
	p.trans.conn.readTimeout = timeout
	p.trans.conn.writeTimeout = timeout
}

func (p *headerProtocol) resetProtocol() error {
	if p.Format != nil && p.protoID == p.trans.ProtocolID() {
		return nil
	}

	p.protoID = p.trans.ProtocolID()
	switch p.protoID {
	case ProtocolIDBinary:
		// These defaults match cpp implementation
		p.Format = NewBinaryProtocol(p.trans, false, true)
	case ProtocolIDCompact:
		p.Format = NewCompactProtocol(p.trans)
	default:
		return NewProtocolException(fmt.Errorf("Unknown protocol id: %#x", p.protoID))
	}
	return nil
}

//
// Writing methods.
//

func (p *headerProtocol) WriteMessageBegin(name string, typeId MessageType, seqid int32) error {
	if err := p.resetProtocol(); err != nil {
		return err
	}
	// The conditions here only match on the Go client side.
	// If we are a client, set header seq id same as msg id
	if typeId == CALL || typeId == ONEWAY {
		p.trans.SetSeqID(uint32(seqid))
	}
	return p.Format.WriteMessageBegin(name, typeId, seqid)
}

//
// Reading methods.
//

func (p *headerProtocol) ReadMessageBegin() (name string, typeId MessageType, seqid int32, err error) {
	if typeId == INVALID_MESSAGE_TYPE {
		if err = p.trans.ResetProtocol(); err != nil {
			return name, EXCEPTION, seqid, err
		}
	}
	err = p.resetProtocol()
	if err != nil {
		return name, EXCEPTION, seqid, err
	}
	// see https://github.com/apache/thrift/blob/master/doc/specs/SequenceNumbers.md
	// TODO:  This is a bug. if we are speaking header protocol, we should be using
	// seq id from the header. However, doing it here creates a non-backwards
	// compatible code between client and server, since they both use this code.
	return p.Format.ReadMessageBegin()
}

func (p *headerProtocol) Flush() (err error) {
	return NewProtocolException(p.trans.Flush())
}

func (p *headerProtocol) Skip(fieldType Type) (err error) {
	return SkipDefaultDepth(p, fieldType)
}

func (p *headerProtocol) Close() error {
	return p.trans.Close()
}

// Deprecated: SetSeqID() is a deprecated method.
func (p *headerProtocol) SetSeqID(seq uint32) {
	p.trans.SetSeqID(seq)
}

// Deprecated: GetSeqID() is a deprecated method.
func (p *headerProtocol) GetSeqID() uint32 {
	return p.trans.SeqID()
}

// Control underlying header transport

func (p *headerProtocol) SetPersistentHeader(key, value string) {
	p.trans.SetPersistentHeader(key, value)
}

func (p *headerProtocol) GetPersistentHeader(key string) (string, bool) {
	return p.trans.GetPersistentHeader(key)
}

func (p *headerProtocol) GetPersistentHeaders() map[string]string {
	return p.trans.GetPersistentHeaders()
}

// Deprecated: SetRequestHeader is deprecated and will eventually be private.
func (p *headerProtocol) SetRequestHeader(key, value string) {
	p.trans.SetRequestHeader(key, value)
}

func (p *headerProtocol) GetRequestHeaders() map[string]string {
	return p.trans.getRequestHeaders()
}

func (p *headerProtocol) GetResponseHeader(key string) (string, bool) {
	return p.trans.GetResponseHeader(key)
}

func (p *headerProtocol) GetResponseHeaders() map[string]string {
	return p.trans.GetResponseHeaders()
}

func (p *headerProtocol) ProtocolID() ProtocolID {
	return p.protoID
}

func (p *headerProtocol) SetProtocolID(protoID ProtocolID) error {
	if err := p.trans.SetProtocolID(protoID); err != nil {
		return err
	}
	return p.resetProtocol()
}

// Deprecated: GetFlags() is a deprecated method.
func (t *headerProtocol) GetFlags() HeaderFlags {
	return t.trans.GetFlags()
}

// Deprecated: SetFlags() is a deprecated method.
func (p *headerProtocol) SetFlags(flags HeaderFlags) {
	p.trans.SetFlags(flags)
}

func (p *headerProtocol) AddTransform(trans TransformID) error {
	return p.trans.AddTransform(trans)
}

// Deprecated: HeaderProtocolSeqID is a deprecated type, temporarily introduced to ease transition to new API.
type HeaderProtocolSeqID interface {
	GetSeqID() uint32
	SetSeqID(uint32)
}

// Compile time interface enforcer
var _ HeaderProtocolSeqID = (*headerProtocol)(nil)

// Deprecated: HeaderProtocolFlags is a deprecated type, temporarily introduced to ease transition to new API.
type HeaderProtocolFlags interface {
	GetFlags() HeaderFlags
	SetFlags(flags HeaderFlags)
}

// Compile time interface enforcer
var _ HeaderProtocolFlags = (*headerProtocol)(nil)
