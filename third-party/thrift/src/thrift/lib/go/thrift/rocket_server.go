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
	request, err := decodeRequestPayload(msg)
	if err != nil {
		return mono.Error(err)
	}
	protocol, err := newProtocolBufferFromRequest(request)
	if err != nil {
		return mono.Error(err)
	}
	if err := processContext(r.ctx, r.proc, protocol); err != nil {
		return mono.Error(err)
	}
	response, err := encodeResponsePayload(protocol.GetRequestHeaders(), request.Zstd(), protocol.Bytes())
	if err != nil {
		return mono.Error(err)
	}
	return mono.Just(response)
}

func newProtocolBufferFromRequest(request *requestPayload) (*protocolBuffer, error) {
	if !request.HasMetadata() {
		return nil, fmt.Errorf("expected metadata")
	}
	protocol, err := newProtocolBuffer(request.Headers(), request.ProtoID(), request.Data())
	if err != nil {
		return nil, err
	}
	if err := protocol.WriteMessageBegin(request.Name(), request.TypeID(), 0); err != nil {
		return nil, err
	}
	return protocol, nil
}
