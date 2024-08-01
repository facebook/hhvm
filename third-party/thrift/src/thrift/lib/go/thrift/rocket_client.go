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
	"maps"
	"net"
	"time"

	rsocket "github.com/rsocket/rsocket-go"
	"github.com/rsocket/rsocket-go/core/transport"
	"github.com/rsocket/rsocket-go/payload"
)

type rocketClient struct {
	Encoder
	Decoder

	conn net.Conn

	// rsocket client state
	ctx    context.Context
	cancel func()
	client rsocket.Client

	resultVal payload.Payload
	resultErr error

	timeout time.Duration

	protoID ProtocolID
	zstd    bool

	messageName string
	writeType   MessageType
	seqID       int32

	reqHeaders        map[string]string
	respHeaders       map[string]string
	persistentHeaders map[string]string

	rbuf *MemoryBuffer
	wbuf *MemoryBuffer
}

// newRocketClient creates a RocketClient
func newRocketClient(conn net.Conn, protoID ProtocolID, timeout time.Duration, persistentHeaders map[string]string) (Protocol, error) {
	p := &rocketClient{
		conn:              conn,
		protoID:           protoID,
		persistentHeaders: persistentHeaders,
		rbuf:              NewMemoryBuffer(),
		wbuf:              NewMemoryBuffer(),
		timeout:           timeout,
		zstd:              false, // zstd adds a performance overhead, so we default to false
	}
	switch p.protoID {
	case ProtocolIDBinary:
		p.Decoder = newBinaryDecoder(p.rbuf)
		p.Encoder = newBinaryEncoder(p.wbuf)
	case ProtocolIDCompact:
		p.Decoder = newCompactDecoder(p.rbuf)
		p.Encoder = newCompactEncoder(p.wbuf)
	default:
		return nil, NewProtocolException(fmt.Errorf("Unknown protocol id: %#x", p.protoID))
	}
	return p, nil
}

func (p *rocketClient) WriteMessageBegin(name string, typeID MessageType, seqid int32) error {
	p.wbuf.Reset()
	p.seqID = seqid
	p.writeType = typeID
	p.messageName = name
	return nil
}

func (p *rocketClient) WriteMessageEnd() error {
	return nil
}

func (p *rocketClient) Flush() (err error) {
	dataBytes := p.wbuf.Bytes()
	request, err := encodeRequestPayload(p.messageName, p.protoID, p.writeType, p.reqHeaders, p.persistentHeaders, p.zstd, dataBytes)
	if err != nil {
		return err
	}

	if err := p.open(); err != nil {
		return err
	}

	// It is necessary to reset the deadline to 0.
	// The rsocket library only sets the deadline at connection start.
	// This means if you wait long enough, the connection will become useless.
	// Or something else is happening, but this is very necessary.
	p.conn.SetDeadline(time.Time{})

	if p.writeType == ONEWAY {
		p.client.FireAndForget(request)
		return nil
	}
	if p.writeType != CALL {
		return nil
	}
	mono := p.client.RequestResponse(request)

	ctx := p.ctx
	if p.timeout > 0 {
		var cancel context.CancelFunc
		ctx, cancel = context.WithTimeout(ctx, p.timeout)
		defer cancel()
	}
	val, err := mono.Block(ctx)
	p.resultVal = payload.Clone(val)
	p.resultErr = err
	return nil
}

// Open opens the internal transport (required for Transport)
func (p *rocketClient) open() error {
	if p.client != nil {
		return nil
	}
	setupPayload, err := newRequestSetupPayloadVersion8()
	if err != nil {
		return err
	}
	p.ctx, p.cancel = context.WithCancel(context.Background())
	clientBuilder := rsocket.Connect()
	// See T182939211. This copies the keep alives from Java Rocket.
	// KeepaliveLifetime = time.Duration(missedAcks = 1) * (ackTimeout = 3600000)
	clientBuilder = clientBuilder.KeepAlive(time.Millisecond*30000, time.Millisecond*3600000, 1)
	clientBuilder = clientBuilder.SetupPayload(setupPayload)
	clientBuilder = clientBuilder.OnClose(p.onClose)
	clientStarter := clientBuilder.Acceptor(p.acceptor)
	p.client, err = clientStarter.Transport(p.transporter).Start(p.ctx)
	return err
}

func (p *rocketClient) transporter(_ context.Context) (*transport.Transport, error) {
	conn := transport.NewTCPClientTransport(p.conn)
	conn.SetLifetime(time.Millisecond * 3600000)
	return conn, nil
}

func (p *rocketClient) onClose(_ error) {
	p.cancel()
}

func (p *rocketClient) acceptor(_ context.Context, socket rsocket.RSocket) rsocket.RSocket {
	return rsocket.NewAbstractSocket(rsocket.MetadataPush(p.serverMetadataPush))
}

func (p *rocketClient) serverMetadataPush(pay payload.Payload) {
	metadata, err := decodeServerMetadataPushVersion8(pay)
	if err != nil {
		panic(err)
	}
	// zstd is only supported if both the client and the server support it.
	p.zstd = p.zstd && metadata.zstd
	if metadata.drain {
		p.Close()
	}
}

func (p *rocketClient) ReadMessageBegin() (string, MessageType, int32, error) {
	name := p.messageName
	if p.resultErr != nil {
		return name, EXCEPTION, p.seqID, p.resultErr
	}
	response, err := decodeResponsePayload(p.resultVal)
	p.respHeaders = make(map[string]string)
	maps.Copy(p.respHeaders, response.Headers())
	if err != nil {
		return name, EXCEPTION, p.seqID, err
	}

	p.rbuf.Init(response.Data())
	return name, REPLY, p.seqID, err
}

func (p *rocketClient) ReadMessageEnd() error {
	return nil
}

func (p *rocketClient) Skip(fieldType Type) (err error) {
	return SkipDefaultDepth(p, fieldType)
}

func (p *rocketClient) SetRequestHeader(key, value string) {
	if p.reqHeaders == nil {
		p.reqHeaders = make(map[string]string)
	}
	p.reqHeaders[key] = value
}

func (p *rocketClient) GetResponseHeaders() map[string]string {
	return p.respHeaders
}

func (p *rocketClient) Close() error {
	if p.client != nil {
		if err := p.client.Close(); err != nil {
			return err
		}
		p.client = nil
	} else {
		p.conn.Close()
	}
	if p.cancel != nil {
		p.cancel()
	}
	return nil
}
