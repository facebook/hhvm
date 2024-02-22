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
)

type HeaderProtocol struct {
	Protocol
	origTransport Transport
	trans         *HeaderTransport

	protoID ProtocolID
}

type HeaderProtocolFactory struct{}

func NewHeaderProtocolFactory() *HeaderProtocolFactory {
	return &HeaderProtocolFactory{}
}

func (p *HeaderProtocolFactory) GetProtocol(trans Transport) Protocol {
	return NewHeaderProtocol(trans)
}

func NewHeaderProtocol(trans Transport) *HeaderProtocol {
	p := &HeaderProtocol{
		origTransport: trans,
		protoID:       ProtocolIDCompact,
	}
	if et, ok := trans.(*HeaderTransport); ok {
		p.trans = et
	} else {
		p.trans = NewHeaderTransport(trans)
	}

	// Effectively an invariant violation.
	if err := p.ResetProtocol(); err != nil {
		panic(err)
	}
	return p
}

func (p *HeaderProtocol) ResetProtocol() error {
	if p.Protocol != nil && p.protoID == p.trans.ProtocolID() {
		return nil
	}

	p.protoID = p.trans.ProtocolID()
	switch p.protoID {
	case ProtocolIDBinary:
		// These defaults match cpp implementation
		p.Protocol = NewBinaryProtocol(p.trans, false, true)
	case ProtocolIDCompact:
		p.Protocol = NewCompactProtocol(p.trans)
	default:
		return NewProtocolException(fmt.Errorf("Unknown protocol id: %#x", p.protoID))
	}
	return nil
}

//
// Writing methods.
//

func (p *HeaderProtocol) WriteMessageBegin(name string, typeId MessageType, seqid int32) error {
	p.ResetProtocol()

	// The conditions here only match on the Go client side.
	// If we are a client, set header seq id same as msg id
	if typeId == CALL || typeId == ONEWAY {
		p.trans.SetSeqID(uint32(seqid))
	}
	return p.Protocol.WriteMessageBegin(name, typeId, seqid)
}

//
// Reading methods.
//

func (p *HeaderProtocol) ReadMessageBegin() (name string, typeId MessageType, seqid int32, err error) {
	if typeId == INVALID_MESSAGE_TYPE {
		if err = p.trans.ResetProtocol(); err != nil {
			return name, EXCEPTION, seqid, err
		}
	}

	err = p.ResetProtocol()
	if err != nil {
		return name, EXCEPTION, seqid, err
	}

	// see https://github.com/apache/thrift/blob/master/doc/specs/SequenceNumbers.md
	// TODO:  This is a bug. if we are speaking header protocol, we should be using
	// seq id from the header. However, doing it here creates a non-backwards
	// compatible code between client and server, since they both use this code.
	return p.Protocol.ReadMessageBegin()
}

func (p *HeaderProtocol) Flush() (err error) {
	return NewProtocolException(p.trans.Flush())
}

func (p *HeaderProtocol) Skip(fieldType Type) (err error) {
	return SkipDefaultDepth(p, fieldType)
}

// Deprecated: Transport() is a deprecated method.
func (p *HeaderProtocol) Transport() Transport {
	return p.origTransport
}

func (p *HeaderProtocol) Close() error {
	return p.origTransport.Close()
}

// Deprecated: SetSeqID() is a deprecated method.
func (p *HeaderProtocol) SetSeqID(seq uint32) {
	p.trans.SetSeqID(seq)
}

// Deprecated: GetSeqID() is a deprecated method.
func (p *HeaderProtocol) GetSeqID() uint32 {
	return p.trans.SeqID()
}

// Control underlying header transport

func (p *HeaderProtocol) SetIdentity(identity string) {
	p.trans.SetIdentity(identity)
}

func (p *HeaderProtocol) Identity() string {
	return p.trans.Identity()
}

func (p *HeaderProtocol) peerIdentity() string {
	return p.trans.peerIdentity()
}

func (p *HeaderProtocol) SetPersistentHeader(key, value string) {
	p.trans.SetPersistentHeader(key, value)
}

func (p *HeaderProtocol) PersistentHeader(key string) (string, bool) {
	return p.trans.PersistentHeader(key)
}

func (p *HeaderProtocol) PersistentHeaders() map[string]string {
	return p.trans.PersistentHeaders()
}

func (p *HeaderProtocol) ClearPersistentHeaders() {
	p.trans.ClearPersistentHeaders()
}

func (p *HeaderProtocol) SetHeader(key, value string) {
	p.trans.SetHeader(key, value)
}

func (p *HeaderProtocol) Header(key string) (string, bool) {
	return p.trans.Header(key)
}

func (p *HeaderProtocol) Headers() map[string]string {
	return p.trans.Headers()
}

func (p *HeaderProtocol) ClearHeaders() {
	p.trans.ClearHeaders()
}

func (p *HeaderProtocol) ReadHeader(key string) (string, bool) {
	return p.trans.ReadHeader(key)
}

func (p *HeaderProtocol) ReadHeaders() map[string]string {
	return p.trans.ReadHeaders()
}

func (p *HeaderProtocol) ProtocolID() ProtocolID {
	return p.protoID
}

// Deprecated: GetFlags() is a deprecated method.
func (t *HeaderProtocol) GetFlags() HeaderFlags {
	return t.trans.GetFlags()
}

func (p *HeaderProtocol) AddTransform(trans TransformID) error {
	return p.trans.AddTransform(trans)
}
