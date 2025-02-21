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
	"runtime"
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

type rocketClient struct {
	types.Encoder
	types.Decoder

	// rsocket client state
	client RSocketClient

	resultData []byte
	resultErr  error

	ioTimeout time.Duration

	protoID types.ProtocolID

	messageName string
	writeType   types.MessageType
	seqID       int32

	reqHeaders        map[string]string
	respHeaders       map[string]string
	persistentHeaders map[string]string

	rbuf *MemoryBuffer
	wbuf *MemoryBuffer
}

var _ Protocol = (*rocketClient)(nil)

// NewRocketClient creates a new Rocket client given an RSocketClient.
func NewRocketClient(client RSocketClient, protoID types.ProtocolID, ioTimeout time.Duration, persistentHeaders map[string]string) (Protocol, error) {
	return newRocketClientFromRsocket(client, protoID, ioTimeout, persistentHeaders)
}

func newRocketClient(conn net.Conn, protoID types.ProtocolID, ioTimeout time.Duration, persistentHeaders map[string]string) (Protocol, error) {
	return newRocketClientFromRsocket(newRSocketClient(conn), protoID, ioTimeout, persistentHeaders)
}

func newRocketClientFromRsocket(client RSocketClient, protoID types.ProtocolID, ioTimeout time.Duration, persistentHeaders map[string]string) (Protocol, error) {
	p := &rocketClient{
		client:            client,
		protoID:           protoID,
		persistentHeaders: persistentHeaders,
		rbuf:              NewMemoryBuffer(),
		wbuf:              NewMemoryBuffer(),
		ioTimeout:         ioTimeout,
		reqHeaders:        make(map[string]string),
	}
	switch p.protoID {
	case types.ProtocolIDBinary:
		p.Decoder = newBinaryDecoder(p.rbuf)
		p.Encoder = newBinaryEncoder(p.wbuf)
	case types.ProtocolIDCompact:
		p.Decoder = newCompactDecoder(p.rbuf)
		p.Encoder = newCompactEncoder(p.wbuf)
	default:
		return nil, types.NewProtocolException(fmt.Errorf("Unknown protocol id: %d", p.protoID))
	}
	runtime.SetFinalizer(p, (*rocketClient).Close)
	return p, nil
}

func (p *rocketClient) WriteMessageBegin(name string, typeID types.MessageType, seqid int32) error {
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

	ctx := context.Background()
	if p.ioTimeout > 0 {
		var cancel context.CancelFunc
		ctx, cancel = context.WithTimeout(ctx, p.ioTimeout)
		defer cancel()
	}

	if err := p.client.SendSetup(ctx); err != nil {
		return err
	}
	headers := unionMaps(p.reqHeaders, p.persistentHeaders)
	if p.writeType == types.ONEWAY {
		return p.client.FireAndForget(p.messageName, p.protoID, headers, dataBytes)
	}
	if p.writeType != types.CALL {
		return nil
	}
	p.respHeaders, p.resultData, p.resultErr = p.client.RequestResponse(ctx, p.messageName, p.protoID, headers, dataBytes)
	clear(p.reqHeaders)
	return nil
}

func unionMaps(dst, src map[string]string) map[string]string {
	if dst == nil {
		return src
	}
	if src == nil {
		return dst
	}
	maps.Copy(dst, src)
	return dst
}

func (p *rocketClient) ReadMessageBegin() (string, types.MessageType, int32, error) {
	name := p.messageName
	if p.resultErr != nil {
		return name, types.EXCEPTION, p.seqID, p.resultErr
	}

	p.rbuf.Init(p.resultData)
	return name, types.REPLY, p.seqID, nil
}

func (p *rocketClient) ReadMessageEnd() error {
	return nil
}

func (p *rocketClient) Skip(fieldType types.Type) (err error) {
	return types.SkipDefaultDepth(p, fieldType)
}

func (p *rocketClient) setRequestHeader(key, value string) {
	p.reqHeaders[key] = value
}

func (p *rocketClient) GetResponseHeaders() map[string]string {
	return p.respHeaders
}

func (p *rocketClient) Close() error {
	// no need for a finalizer anymore
	runtime.SetFinalizer(p, nil)
	return p.client.Close()
}

func (p *rocketClient) DO_NOT_USE_WrapChannel() RequestChannel {
	return NewSerialChannel(p)
}
