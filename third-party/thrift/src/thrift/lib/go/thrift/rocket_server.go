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
	"sync/atomic"
	"time"

	"github.com/jjeffcaii/reactor-go/scheduler"
	rsocket "github.com/rsocket/rsocket-go"
	"github.com/rsocket/rsocket-go/core/transport"
	"github.com/rsocket/rsocket-go/payload"
	"github.com/rsocket/rsocket-go/rx/mono"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/rocket"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/stats"
)

type rocketServer struct {
	proc          Processor
	listener      net.Listener
	transportID   TransportID
	zstdSupported bool
	log           func(format string, args ...any)
	connContext   ConnContextFunc

	pipeliningEnabled bool
	numWorkers        int

	stats                   *stats.ServerStats
	pstats                  map[string]*stats.TimingSeries
	observer                ServerObserver
	totalActiveRequestCount atomic.Int64
}

func newRocketServer(proc Processor, listener net.Listener, opts *serverOptions) Server {
	rocket.SetRsocketLogger(opts.log)
	return &rocketServer{
		proc:          proc,
		listener:      listener,
		transportID:   TransportIDRocket,
		zstdSupported: true,
		log:           opts.log,
		connContext:   opts.connContext,

		pipeliningEnabled: opts.pipeliningEnabled,
		numWorkers:        opts.numWorkers,

		pstats:   opts.processorStats,
		stats:    opts.serverStats,
		observer: opts.serverObserver,
	}
}

func newUpgradeToRocketServer(proc Processor, listener net.Listener, opts *serverOptions) Server {
	rocket.SetRsocketLogger(opts.log)
	return &rocketServer{
		proc:          proc,
		listener:      listener,
		transportID:   TransportIDUpgradeToRocket,
		zstdSupported: true,
		log:           opts.log,
		connContext:   opts.connContext,

		pipeliningEnabled: opts.pipeliningEnabled,
		numWorkers:        opts.numWorkers,

		pstats:   opts.processorStats,
		stats:    opts.serverStats,
		observer: opts.serverObserver,
	}
}

func (s *rocketServer) ServeContext(ctx context.Context) error {
	transporter := func(context.Context) (transport.ServerTransport, error) {
		return newRocketServerTransport(s.listener, s.connContext, s.proc, s.transportID, s.log, s.stats, s.pstats, s.observer), nil
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
	if err := rocket.CheckRequestSetupMetadata8(setup); err != nil {
		return nil, err
	}
	serverMetadataPush, err := rocket.EncodeServerMetadataPush(s.zstdSupported)
	if err != nil {
		return nil, err
	}
	sendingSocket.MetadataPush(serverMetadataPush)
	socket := newRocketServerSocket(ctx, s.proc, s.pipeliningEnabled, s.log, s.stats, s.pstats, s.observer, &s.totalActiveRequestCount)
	return rsocket.NewAbstractSocket(
		rsocket.MetadataPush(socket.metadataPush),
		rsocket.RequestResponse(socket.requestResonse),
		rsocket.FireAndForget(socket.fireAndForget),
	), nil
}

type rocketServerSocket struct {
	ctx                     context.Context
	proc                    Processor
	pipeliningEnabled       bool
	log                     func(format string, args ...any)
	stats                   *stats.ServerStats
	pstats                  map[string]*stats.TimingSeries
	observer                ServerObserver
	totalActiveRequestCount *atomic.Int64
}

func newRocketServerSocket(
	ctx context.Context,
	proc Processor,
	pipeliningEnabled bool,
	log func(format string, args ...any),
	stats *stats.ServerStats,
	pstats map[string]*stats.TimingSeries,
	observer ServerObserver,
	totalActiveRequestCount *atomic.Int64,
) *rocketServerSocket {
	return &rocketServerSocket{
		ctx:                     ctx,
		proc:                    proc,
		pipeliningEnabled:       pipeliningEnabled,
		log:                     log,
		stats:                   stats,
		pstats:                  pstats,
		observer:                observer,
		totalActiveRequestCount: totalActiveRequestCount,
	}
}

// incrementActiveRequests increments the server-level active request counter and
// notifies the observer with the current total count across all sockets
func (s *rocketServerSocket) incrementActiveRequests() {
	current := s.totalActiveRequestCount.Add(1)
	s.observer.ActiveRequests(int(current))
}

// decrementActiveRequests decrements the server-level active request counter and
// notifies the observer with the current total count across all sockets
func (s *rocketServerSocket) decrementActiveRequests() {
	current := s.totalActiveRequestCount.Add(-1)
	s.observer.ActiveRequests(int(current))
}

func (s *rocketServerSocket) metadataPush(msg payload.Payload) {
	_, err := rocket.DecodeClientMetadataPush(msg)
	if err != nil {
		panic(err)
	}
	// This is usually something like transportMetadata = map[deciding_accessors:IP=...], but we do not handle it.
}

func (s *rocketServerSocket) requestResonse(msg payload.Payload) mono.Mono {
	requestReceivedTime := time.Now()

	request, err := rocket.DecodeRequestPayload(msg)
	if err != nil {
		// Notify observer that connection was dropped due to malformed rocket payload
		s.observer.ConnDropped()
		return mono.Error(err)
	}
	protocol, err := newProtocolBufferFromRequest(request)
	if err != nil {
		// Notify observer that connection was dropped due to protocol buffer creation error
		s.observer.ConnDropped()
		return mono.Error(err)
	}

	// Notify observer that request was received
	s.observer.ReceivedRequest()

	s.stats.SchedulingWorkCount.Incr()
	workItem := func(ctx context.Context) (payload.Payload, error) {
		// Increment active requests when processing actually starts
		s.incrementActiveRequests()
		defer s.decrementActiveRequests()

		s.stats.SchedulingWorkCount.Decr()
		s.stats.WorkingCount.Incr()
		defer s.stats.WorkingCount.Decr()

		// Track process delay from request received to processing start
		processStartTime := time.Now()
		processDelay := processStartTime.Sub(requestReceivedTime)
		s.observer.ProcessDelay(processDelay)

		// Track actual handler execution time
		if err := process(ctx, s.proc, protocol, s.pstats); err != nil {
			// Notify observer that connection was dropped due to unparseable message begin
			s.observer.ConnDropped()
			return nil, err
		}

		protocol.setRequestHeader(LoadHeaderKey, fmt.Sprintf("%d", loadFn(s.stats)))

		payload, err := rocket.EncodeResponsePayload(
			protocol.name,
			protocol.messageType,
			protocol.getRequestHeaders(),
			request.GetCompressionForResponse(),
			protocol.Bytes(),
		)

		// Track actual handler execution time
		processTime := time.Since(processStartTime)
		s.observer.ProcessTime(processTime)

		// Notify observer that reply was sent
		if err == nil {
			s.observer.SentReply()
		}

		return payload, err
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
	requestReceivedTime := time.Now()

	request, err := rocket.DecodeRequestPayload(msg)
	if err != nil {
		// Notify observer that connection was dropped due to malformed rocket payload
		s.observer.ConnDropped()
		s.log("rocketServer fireAndForget decode request payload error: %v", err)
		return
	}
	protocol, err := newProtocolBufferFromRequest(request)
	if err != nil {
		// Notify observer that connection was dropped due to protocol buffer creation error
		s.observer.ConnDropped()
		s.log("rocketServer fireAndForget error creating protocol: %v", err)
		return
	}

	// Notify observer that request was received
	s.observer.ReceivedRequest()

	// Track process delay from request received to processing start
	processStartTime := time.Now()
	processDelay := processStartTime.Sub(requestReceivedTime)
	s.observer.ProcessDelay(processDelay)

	// Increment active requests when processing starts
	s.incrementActiveRequests()
	defer s.decrementActiveRequests()

	// TODO: support pipelining
	if err := process(s.ctx, s.proc, protocol, s.pstats); err != nil {
		// Notify observer that connection was dropped due to unparseable message begin
		s.observer.ConnDropped()
		s.log("rocketServer fireAndForget process error: %v", err)
		return
	}

	// Track actual handler execution time
	processTime := time.Since(processStartTime)
	s.observer.ProcessTime(processTime)
}

func newProtocolBufferFromRequest(request *rocket.RequestPayload) (*protocolBuffer, error) {
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
