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
	"errors"
	"fmt"
	"math"
	"net"
	"sync/atomic"
	"time"

	"github.com/jjeffcaii/reactor-go/scheduler"
	rsocket "github.com/rsocket/rsocket-go"
	"github.com/rsocket/rsocket-go/core/transport"
	"github.com/rsocket/rsocket-go/payload"
	"github.com/rsocket/rsocket-go/rx/flux"
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

	numWorkers int

	stats                   *stats.ServerStats
	pstats                  map[string]*stats.TimingSeries
	observer                ServerObserver
	maxRequests             int64
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

		numWorkers: opts.numWorkers,

		pstats:      opts.processorStats,
		stats:       opts.serverStats,
		observer:    opts.serverObserver,
		maxRequests: opts.maxRequests,
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

		numWorkers: opts.numWorkers,

		pstats:      opts.processorStats,
		stats:       opts.serverStats,
		observer:    opts.serverObserver,
		maxRequests: opts.maxRequests,
	}
}

func (s *rocketServer) ServeContext(ctx context.Context) error {
	// TODO: support graceful shutdown and track with thrift.task_killed

	transporter := func(context.Context) (transport.ServerTransport, error) {
		return newRocketServerTransport(s.listener, s.connContext, s.proc, s.transportID, s.log, s.stats, s.pstats, s.observer), nil
	}

	// Request scheduler must be elastic to ensure that we can quickly peek at
	// the request metadata and set the necessary timeouts (e.g. queue timeout).
	// The actual heavy lifting (e.g. request processing, handling, response
	// serializaton) will be done by the response scheduler.
	requestScheduler := scheduler.NewElastic(math.MaxInt32)
	var responseScheduler scheduler.Scheduler
	if s.numWorkers == GoroutinePerRequest {
		responseScheduler = scheduler.NewElastic(math.MaxInt32)
	} else {
		responseScheduler = scheduler.NewElastic(s.numWorkers)
	}
	defer requestScheduler.Close()
	defer responseScheduler.Close()

	r := rsocket.Receive().
		Scheduler(requestScheduler, responseScheduler).
		Acceptor(s.acceptor).
		Transport(transporter)
	return r.Serve(ctx)
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
	socket := newRocketServerSocket(
		ctx,
		s.proc,
		s.log,
		s.stats,
		s.pstats,
		s.observer,
		s.maxRequests,
		&s.totalActiveRequestCount,
	)
	return rsocket.NewAbstractSocket(
		rsocket.MetadataPush(socket.metadataPush),
		rsocket.RequestResponse(socket.requestResponse),
		rsocket.FireAndForget(socket.fireAndForget),
		rsocket.RequestStream(socket.requestStream),
	), nil
}

type rocketServerSocket struct {
	ctx                     context.Context
	proc                    Processor
	log                     func(format string, args ...any)
	stats                   *stats.ServerStats
	pstats                  map[string]*stats.TimingSeries
	observer                ServerObserver
	maxRequests             int64
	totalActiveRequestCount *atomic.Int64
}

func newRocketServerSocket(
	ctx context.Context,
	proc Processor,
	log func(format string, args ...any),
	stats *stats.ServerStats,
	pstats map[string]*stats.TimingSeries,
	observer ServerObserver,
	maxRequests int64,
	totalActiveRequestCount *atomic.Int64,
) *rocketServerSocket {
	return &rocketServerSocket{
		ctx:                     ctx,
		proc:                    proc,
		log:                     log,
		stats:                   stats,
		pstats:                  pstats,
		observer:                observer,
		maxRequests:             maxRequests,
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

// isOverloaded checks if adding one more request would exceed the configured maxRequests limit
// Returns true if the server should reject new requests due to high load
//
// TODO: align with C++ implementation
func (s *rocketServerSocket) isOverloaded() bool {
	// If maxRequests is 0 (default), overload protection is disabled
	if s.maxRequests == 0 {
		return false
	}
	countWithNewRequest := int64(s.totalActiveRequestCount.Load()) + 1
	return countWithNewRequest > s.maxRequests
}

func (s *rocketServerSocket) metadataPush(msg payload.Payload) {
	_, err := rocket.DecodeClientMetadataPush(msg)
	if err != nil {
		panic(err)
	}
	// This is usually something like transportMetadata = map[deciding_accessors:IP=...], but we do not handle it.
}

func (s *rocketServerSocket) requestResponse(msg payload.Payload) mono.Mono {
	// TODO: this clone helps prevent a race-condition where the payload gets
	// released by the underlying rsocket layer before we are done with it.
	msg = payload.Clone(msg)

	requestReceivedTime := time.Now()

	_, request, err := rocket.DecodeRequestPayload(msg)
	if err != nil {
		// Notify observer that connection was dropped and task killed due to malformed rocket payload
		s.observer.ConnDropped()
		s.observer.TaskKilled()
		return mono.Error(err)
	}
	protocol, err := newProtocolBufferFromRequest(request)
	if err != nil {
		// Notify observer that connection was dropped and task killed due to protocol buffer creation error
		s.observer.ConnDropped()
		s.observer.TaskKilled()
		return mono.Error(err)
	}

	// Notify observer that request was received
	s.observer.ReceivedRequest()
	s.observer.ReceivedRequestForFunction(protocol.name)

	if s.isOverloaded() {
		// Track connection drops and server overload events when rejecting requests
		s.observer.ConnDropped()
		s.observer.ServerOverloaded()
		return mono.Error(loadSheddingError)
	}

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
		if err := process(ctx, s.proc, protocol, s.pstats, s.observer); err != nil {
			// Notify observer that connection was dropped due to unparseable message begin
			s.observer.ConnDropped()
			return nil, err
		}

		protocol.setRequestHeader(LoadHeaderKey, fmt.Sprintf("%d", loadFn(s.stats, s.totalActiveRequestCount)))

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
	return mono.FromFunc(workItem)
}

func (s *rocketServerSocket) fireAndForget(msg payload.Payload) {
	// TODO: this clone helps prevent a race-condition where the payload gets
	// released by the underlying rsocket layer before we are done with it.
	msg = payload.Clone(msg)

	requestReceivedTime := time.Now()

	_, request, err := rocket.DecodeRequestPayload(msg)
	if err != nil {
		// Notify observer that connection was dropped and task killed due to malformed rocket payload
		s.observer.ConnDropped()
		s.observer.TaskKilled()
		s.log("rocketServer fireAndForget decode request payload error: %v", err)
		return
	}
	protocol, err := newProtocolBufferFromRequest(request)
	if err != nil {
		// Notify observer that connection was dropped and task killed due to protocol buffer creation error
		s.observer.ConnDropped()
		s.observer.TaskKilled()
		s.log("rocketServer fireAndForget error creating protocol: %v", err)
		return
	}

	// Notify observer that request was received
	s.observer.ReceivedRequest()
	s.observer.ReceivedRequestForFunction(protocol.name)

	if s.isOverloaded() {
		// Track connection drops and server overload events when rejecting requests
		s.observer.ConnDropped()
		s.observer.ServerOverloaded()
		s.log("rocketServer fireAndForget: dropping request due to server overload")
		return
	}

	// Increment active requests when processing starts
	s.incrementActiveRequests()
	defer s.decrementActiveRequests()

	// Track process delay from request received to processing start
	processStartTime := time.Now()
	processDelay := processStartTime.Sub(requestReceivedTime)
	s.observer.ProcessDelay(processDelay)

	// TODO: support pipelining
	if err := process(s.ctx, s.proc, protocol, s.pstats, s.observer); err != nil {
		// Notify observer that connection was dropped due to unparseable message begin
		s.observer.ConnDropped()
		s.log("rocketServer fireAndForget process error: %v", err)
		return
	}

	// Track actual handler execution time
	processTime := time.Since(processStartTime)
	s.observer.ProcessTime(processTime)
}

func (s *rocketServerSocket) requestStream(msg payload.Payload) flux.Flux {
	return flux.Error(errors.New("not implemented"))
}

func newProtocolBufferFromRequest(request *rocket.RequestPayload) (*protocolBuffer, error) {
	protocol, err := newProtocolBuffer(request.ProtoID(), request.Data())
	if err != nil {
		return nil, err
	}
	protocol.setResponseHeaders(request.Headers())
	if err := protocol.WriteMessageBegin(request.Name(), request.TypeID(), 0); err != nil {
		return nil, err
	}
	return protocol, nil
}
