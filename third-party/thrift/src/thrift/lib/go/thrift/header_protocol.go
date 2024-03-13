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

type headerProtocol struct {
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

func NewHeaderProtocol(trans Transport) *headerProtocol {
	p := &headerProtocol{
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

func (p *headerProtocol) ResetProtocol() error {
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

func (p *headerProtocol) WriteMessageBegin(name string, typeId MessageType, seqid int32) error {
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

func (p *headerProtocol) ReadMessageBegin() (name string, typeId MessageType, seqid int32, err error) {
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

func (p *headerProtocol) Flush() (err error) {
	return NewProtocolException(p.trans.Flush())
}

func (p *headerProtocol) Skip(fieldType Type) (err error) {
	return SkipDefaultDepth(p, fieldType)
}

func (p *headerProtocol) Close() error {
	return p.origTransport.Close()
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

func (p *headerProtocol) SetIdentity(identity string) {
	p.trans.SetIdentity(identity)
}

func (p *headerProtocol) Identity() string {
	return p.trans.Identity()
}

func (p *headerProtocol) peerIdentity() string {
	return p.trans.peerIdentity()
}

func (p *headerProtocol) SetPersistentHeader(key, value string) {
	p.trans.SetPersistentHeader(key, value)
}

func (p *headerProtocol) GetPersistentHeader(key string) (string, bool) {
	return p.trans.GetPersistentHeader(key)
}

func (p *headerProtocol) GetPersistentHeaders() map[string]string {
	return p.trans.GetPersistentHeaders()
}

func (p *headerProtocol) ClearPersistentHeaders() {
	p.trans.ClearPersistentHeaders()
}

// GetRequestHeader returns a request header if the key exists, otherwise false
func (p *headerProtocol) GetRequestHeader(key string) (string, bool) {
	return p.trans.GetRequestHeader(key)
}

// Deprecated SetHeader is deprecated, rather use SetRequestHeader
func (p *headerProtocol) SetHeader(key, value string) {
	p.trans.SetRequestHeader(key, value)
}

// Deprecated Header is deprecated, rather use GetRequestHeader
func (p *headerProtocol) Header(key string) (string, bool) {
	return p.trans.GetRequestHeader(key)
}

// Deprecated Headers is deprecated, rather use GetRequestHeaders
func (p *headerProtocol) Headers() map[string]string {
	return p.trans.GetRequestHeaders()
}

// Deprecated: SetRequestHeader is deprecated and will eventually be private.
func (p *headerProtocol) SetRequestHeader(key, value string) {
	p.trans.SetRequestHeader(key, value)
}

// Deprecated: GetRequestHeader is deprecated and will eventually be private.
func (p *headerProtocol) GetRequestHeaders() map[string]string {
	return p.trans.GetRequestHeaders()
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

// Deprecated: HeaderProtocolFlags is a deprecated type, temporarily introduced to ease transition to new API.
type HeaderProtocolFlags interface {
	GetFlags() HeaderFlags
	SetFlags(flags HeaderFlags)
}

// Compile time interface enforcer
var _ HeaderProtocolFlags = (*headerProtocol)(nil)

// Deprecated: HeaderProtocolProtocolID is a deprecated type, temporarily introduced to ease transition to new API.
type HeaderProtocolProtocolID interface {
	ProtocolID() ProtocolID
}

// Compile time interface enforcer
var _ HeaderProtocolProtocolID = (*headerProtocol)(nil)
