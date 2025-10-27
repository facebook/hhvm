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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/format"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

type headerProtocol struct {
	types.Format
	trans *headerTransport

	protoID types.ProtocolID
}

var _ Protocol = (*headerProtocol)(nil)

// NewHeaderProtocol creates a new header protocol.
func NewHeaderProtocol(conn net.Conn) (Protocol, error) {
	return newHeaderProtocol(conn, types.ProtocolIDCompact, 0, nil)
}

func newHeaderProtocolAsRequestChannel(conn net.Conn, protoID types.ProtocolID, ioTimeout time.Duration, persistentHeaders map[string]string) (RequestChannel, error) {
	proto, err := newHeaderProtocol(conn, protoID, ioTimeout, persistentHeaders)
	if err != nil {
		return nil, err
	}
	return newSerialChannel(proto), nil
}

func newHeaderProtocol(conn net.Conn, protoID types.ProtocolID, ioTimeout time.Duration, persistentHeaders map[string]string) (Protocol, error) {
	p := &headerProtocol{protoID: protoID}
	p.trans = newHeaderTransport(conn, protoID)
	p.trans.conn.readTimeout = ioTimeout
	p.trans.conn.writeTimeout = ioTimeout
	if err := p.resetProtocol(); err != nil {
		return nil, err
	}
	for name, value := range persistentHeaders {
		p.trans.persistentWriteInfoHeaders[name] = value
	}
	return p, nil
}

func (p *headerProtocol) resetProtocol() error {
	if p.Format != nil && p.protoID == p.trans.protoID {
		return nil
	}

	p.protoID = p.trans.protoID
	switch p.protoID {
	case types.ProtocolIDBinary:
		// These defaults match cpp implementation
		p.Format = format.NewBinaryFormatOptions(p.trans, false, true)
	case types.ProtocolIDCompact:
		p.Format = format.NewCompactFormat(p.trans)
	default:
		return types.NewProtocolException(fmt.Errorf("Unknown protocol id: %d", p.protoID))
	}
	return nil
}

//
// Writing methods.
//

func (p *headerProtocol) WriteMessageBegin(name string, typeId types.MessageType, seqid int32) error {
	if err := p.resetProtocol(); err != nil {
		return err
	}
	// The conditions here only match on the Go client side.
	// If we are a client, set header seq id same as msg id
	if typeId == types.CALL || typeId == types.ONEWAY {
		p.trans.SetSeqID(uint32(seqid))
	}
	return p.Format.WriteMessageBegin(name, typeId, seqid)
}

//
// Reading methods.
//

func (p *headerProtocol) ReadMessageBegin() (name string, typeId types.MessageType, seqid int32, err error) {
	if typeId == types.INVALID_MESSAGE_TYPE {
		if err = p.trans.ResetProtocol(); err != nil {
			return name, types.EXCEPTION, seqid, err
		}
	}
	err = p.resetProtocol()
	if err != nil {
		return name, types.EXCEPTION, seqid, err
	}
	// see https://github.com/apache/thrift/blob/master/doc/specs/SequenceNumbers.md
	// TODO:  This is a bug. if we are speaking header protocol, we should be using
	// seq id from the header. However, doing it here creates a non-backwards
	// compatible code between client and server, since they both use this code.
	return p.Format.ReadMessageBegin()
}

func (p *headerProtocol) Flush() error {
	return types.NewProtocolException(p.trans.Flush())
}

func (p *headerProtocol) Skip(fieldType types.Type) error {
	return types.SkipDefaultDepth(p, fieldType)
}

func (p *headerProtocol) Close() error {
	return p.trans.Close()
}

// Control underlying header transport

// Deprecated: setRequestHeader is deprecated and will eventually be private.
func (p *headerProtocol) setRequestHeader(key, value string) {
	p.trans.SetRequestHeader(key, value)
}

func (p *headerProtocol) getResponseHeaders() map[string]string {
	return p.trans.GetResponseHeaders()
}
