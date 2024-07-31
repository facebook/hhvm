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
	"context"
	"fmt"
	"net"

	rsocket "github.com/rsocket/rsocket-go"
	"github.com/rsocket/rsocket-go/core/transport"
	"github.com/rsocket/rsocket-go/payload"
	"github.com/rsocket/rsocket-go/rx/mono"
)

type rocketServer struct {
	proc        ProcessorContext
	listener    net.Listener
	transportID TransportID
}

func newRocketServer(proc ProcessorContext, listener net.Listener) Server {
	return &rocketServer{
		proc:        proc,
		listener:    listener,
		transportID: TransportIDRocket,
	}
}

func newUpgradeToRocketServer(proc ProcessorContext, listener net.Listener) Server {
	return &rocketServer{
		proc:        proc,
		listener:    listener,
		transportID: TransportIDUpgradeToRocket,
	}
}

func (s *rocketServer) ServeContext(ctx context.Context) error {
	transporter := func(context.Context) (transport.ServerTransport, error) {
		return newRocketServerTransport(s.listener, s.proc, s.transportID), nil
	}
	r := rsocket.Receive().Acceptor(s.acceptor).Transport(transporter)
	return r.Serve(ctx)
}

func (s *rocketServer) acceptor(ctx context.Context, setup payload.SetupPayload, sendingSocket rsocket.CloseableRSocket) (rsocket.RSocket, error) {
	conn := &rocketServerSocket{ctx: ctx, proc: s.proc}
	return rsocket.NewAbstractSocket(
		rsocket.RequestResponse(conn.requestResonse),
	), nil
}

type rocketServerSocket struct {
	ctx  context.Context
	proc ProcessorContext
}

func (r *rocketServerSocket) requestResonse(msg payload.Payload) mono.Mono {
	msg = payload.Clone(msg)
	reqMetadataBytes, ok := msg.Metadata()
	if !ok {
		return mono.Error(fmt.Errorf("expected metadata"))
	}
	reqMetadata, err := deserializeRequestRPCMetadata(reqMetadataBytes)
	if err != nil {
		return mono.Error(err)
	}
	if reqMetadata.Zstd {
		return mono.Error(fmt.Errorf("currently only supporting uncompressed COMPACT protocol"))
	}
	protocol, err := newBufProtocol(reqMetadata.Name, reqMetadata.TypeID, reqMetadata.Other, reqMetadata.ProtoID, msg.Data())
	if err != nil {
		return mono.Error(err)
	}
	if err := processContext(r.ctx, r.proc, protocol); err != nil {
		return mono.Error(err)
	}
	respMetadata := &responseRPCMetadata{
		Zstd:  reqMetadata.Zstd,
		Other: protocol.reqHeaders,
	}
	respMetadataBytes, err := serializeResponseRPCMetadata(respMetadata)
	if err != nil {
		return mono.Error(err)
	}
	respDataBytes := protocol.Bytes()
	response := payload.New(respDataBytes, respMetadataBytes)
	return mono.Just(response)
}

// bufProtocol is a protocol that is provided with a buffer to read from and writes to an empty buffer that can be retrieved via the Bytes method.
type bufProtocol struct {
	Decoder
	Encoder
	wbuf        *MemoryBuffer
	name        string
	typeID      MessageType
	reqHeaders  map[string]string
	respHeaders map[string]string
}

var _ Protocol = (*bufProtocol)(nil)

func newBufProtocol(name string, typeID MessageType, respHeaders map[string]string, protoID ProtocolID, data []byte) (*bufProtocol, error) {
	wbuf := NewMemoryBuffer()
	p := &bufProtocol{
		Decoder:     newCompactDecoder(NewMemoryBufferWithData(data)),
		Encoder:     newCompactEncoder(wbuf),
		wbuf:        wbuf,
		name:        name,
		typeID:      typeID,
		respHeaders: respHeaders,
		reqHeaders:  map[string]string{},
	}
	rbuf := NewMemoryBufferWithData(data)
	switch protoID {
	case ProtocolIDBinary:
		p.Decoder = newBinaryDecoder(rbuf)
		p.Encoder = newBinaryEncoder(wbuf)
	case ProtocolIDCompact:
		p.Decoder = newCompactDecoder(rbuf)
		p.Encoder = newCompactEncoder(wbuf)
	default:
		return nil, NewProtocolException(fmt.Errorf("Unknown protocol id: %#x", protoID))
	}
	return p, nil
}

func (b *bufProtocol) Bytes() []byte {
	return b.wbuf.Buffer.Bytes()
}

func (b *bufProtocol) ReadMessageBegin() (string, MessageType, int32, error) {
	return b.name, b.typeID, 0, nil
}

func (b *bufProtocol) ReadMessageEnd() error {
	return nil
}

func (b *bufProtocol) WriteMessageBegin(name string, typeID MessageType, seqid int32) error {
	return nil
}

func (b *bufProtocol) WriteMessageEnd() error {
	return nil
}

func (b *bufProtocol) Close() error {
	return nil
}

func (b *bufProtocol) GetResponseHeaders() map[string]string {
	return b.respHeaders
}

func (b *bufProtocol) SetRequestHeader(key, value string) {
	b.reqHeaders[key] = value
}
