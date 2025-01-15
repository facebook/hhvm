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

	"github.com/jjeffcaii/reactor-go/scheduler"
	rsocket "github.com/rsocket/rsocket-go"
	"github.com/rsocket/rsocket-go/core/transport"
	"github.com/rsocket/rsocket-go/payload"
	"github.com/rsocket/rsocket-go/rx/mono"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/stats"
)

type rocketServer struct {
	proc          Processor
	listener      net.Listener
	transportID   TransportID
	zstdSupported bool
	log           func(format string, args ...interface{})
	connContext   ConnContextFunc

	pipeliningEnabled bool
	numWorkers        int

	stats  *stats.ServerStats
	pstats map[string]*stats.TimingSeries
}

func newRocketServer(proc Processor, listener net.Listener, opts *serverOptions) Server {
	setRsocketLogger(opts.log)
	return &rocketServer{
		proc:          proc,
		listener:      listener,
		transportID:   TransportIDRocket,
		zstdSupported: true,
		log:           opts.log,
		connContext:   opts.connContext,

		pipeliningEnabled: opts.pipeliningEnabled,
		numWorkers:        opts.numWorkers,

		pstats: opts.processorStats,
		stats:  opts.serverStats,
	}
}

func newUpgradeToRocketServer(proc Processor, listener net.Listener, opts *serverOptions) Server {
	setRsocketLogger(opts.log)
	return &rocketServer{
		proc:          proc,
		listener:      listener,
		transportID:   TransportIDUpgradeToRocket,
		zstdSupported: true,
		log:           opts.log,
		connContext:   opts.connContext,

		pipeliningEnabled: opts.pipeliningEnabled,
		numWorkers:        opts.numWorkers,

		pstats: opts.processorStats,
		stats:  opts.serverStats,
	}
}

func (s *rocketServer) ServeContext(ctx context.Context) error {
	transporter := func(context.Context) (transport.ServerTransport, error) {
		return newRocketServerTransport(s.listener, s.connContext, s.proc, s.transportID, s.log, s.stats), nil
	}
	r := rsocket.Receive().
		Scheduler(s.requestScheduler(), s.responeScheduler()).
		Acceptor(s.acceptor).
		Transport(transporter)
	return r.Serve(ctx)
}

func (s *rocketServer) requestScheduler() scheduler.Scheduler {
	if s.numWorkers == GoroutinePerRequest {
		return scheduler.Elastic()
	}
	return scheduler.NewElastic(s.numWorkers)
}

func (s *rocketServer) responeScheduler() scheduler.Scheduler {
	return scheduler.Elastic()
}

func (s *rocketServer) acceptor(ctx context.Context, setup payload.SetupPayload, sendingSocket rsocket.CloseableRSocket) (rsocket.RSocket, error) {
	if err := checkRequestSetupMetadata8(setup); err != nil {
		return nil, err
	}
	serverMetadataPush, err := encodeServerMetadataPush(s.zstdSupported)
	if err != nil {
		return nil, err
	}
	sendingSocket.MetadataPush(serverMetadataPush)
	socket := newRocketServerSocket(ctx, s.proc, s.pipeliningEnabled, s.log, s.stats)
	return rsocket.NewAbstractSocket(
		rsocket.MetadataPush(socket.metadataPush),
		rsocket.RequestResponse(socket.requestResonse),
		rsocket.FireAndForget(socket.fireAndForget),
	), nil
}

type rocketServerSocket struct {
	ctx               context.Context
	proc              Processor
	pipeliningEnabled bool
	log               func(format string, args ...interface{})
	stats             *stats.ServerStats
}

func newRocketServerSocket(ctx context.Context, proc Processor, pipeliningEnabled bool, log func(format string, args ...interface{}), stats *stats.ServerStats) *rocketServerSocket {
	return &rocketServerSocket{
		ctx:               ctx,
		proc:              proc,
		pipeliningEnabled: pipeliningEnabled,
		log:               log,
		stats:             stats,
	}
}

func (s *rocketServerSocket) metadataPush(msg payload.Payload) {
	_, err := decodeClientMetadataPush(msg)
	if err != nil {
		panic(err)
	}
	// This is usually something like transportMetadata = map[deciding_accessors:IP=...], but we do not handle it.
}

func (s *rocketServerSocket) requestResonse(msg payload.Payload) mono.Mono {
	request, err := decodeRequestPayload(msg)
	if err != nil {
		return mono.Error(err)
	}
	protocol, err := newProtocolBufferFromRequest(request)
	if err != nil {
		return mono.Error(err)
	}
	s.stats.SchedulingWorkCount.Incr()
	workItem := func(ctx context.Context) (payload.Payload, error) {
		s.stats.SchedulingWorkCount.Decr()
		s.stats.WorkingCount.Incr()
		defer s.stats.WorkingCount.Decr()
		if err := process(ctx, s.proc, protocol); err != nil {
			return nil, err
		}
		protocol.SetRequestHeader(LoadHeaderKey, fmt.Sprintf("%d", loadFn(s.stats)))
		return encodeResponsePayload(protocol.name, protocol.messageType, protocol.getRequestHeaders(), request.Zstd(), protocol.Bytes())
	}
	if s.pipeliningEnabled {
		return mono.FromFunc(workItem)
	}
	response, err := workItem(s.ctx)
	if err != nil {
		return mono.Error(err)
	}
	return mono.Just(response)
}

func (s *rocketServerSocket) fireAndForget(msg payload.Payload) {
	request, err := decodeRequestPayload(msg)
	if err != nil {
		s.log("rocketServer fireAndForget decode request payload error: %v", err)
		return
	}
	protocol, err := newProtocolBufferFromRequest(request)
	if err != nil {
		s.log("rocketServer fireAndForget error creating protocol: %v", err)
		return
	}
	// TODO: support pipelining
	if err := process(s.ctx, s.proc, protocol); err != nil {
		s.log("rocketServer fireAndForget process error: %v", err)
		return
	}
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
