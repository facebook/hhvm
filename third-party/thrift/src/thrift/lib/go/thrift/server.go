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
	"math"
	"net"
	"runtime"
	"strings"
	"sync"
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
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
)

var loadSheddingError = NewApplicationException(
	LOADSHEDDING,
	"load shedding due to max request limit",
)

var taskExpiredError = NewApplicationException(
	UNKNOWN_APPLICATION_EXCEPTION,
	"Task Expired",
)

// Server is a thrift server
type Server interface {
	// ServeContext starts the server, and stops it when the context is cancelled
	ServeContext(ctx context.Context) error
}

// NewServer creates a new thrift server. It includes:
// * load shedding support
// * load balancing compatible with high QPS services
// * pipelining of incoming requests on same connection
// * out of order responses (for clients that support it!)
// * and statstics that you can export to your favorite monitoring system
func NewServer(proc Processor, listener net.Listener, transportType TransportID, options ...ServerOption) Server {
	actualTransportID := TransportIDUpgradeToRocket
	switch transportType {
	case TransportIDHeader:
		// NOTE: temporary workaround while Header support is being removed.
		// This code is never hit actually.
		actualTransportID = TransportIDUpgradeToRocket
	case TransportIDRocket:
		actualTransportID = TransportIDRocket
	case TransportIDUpgradeToRocket:
		actualTransportID = TransportIDUpgradeToRocket
	default:
		panic(fmt.Sprintf("Server does not support: %v", transportType))
	}

	config := newServerConfig(options...)
	rocket.SetRsocketLogger(config.log)
	result := &rocketServer{
		proc:          proc,
		listener:      listener,
		transportID:   actualTransportID,
		zstdSupported: true,
		log:           config.log,
		connContext:   config.connContext,

		numWorkers: config.numWorkers,

		pstats:      config.processorStats,
		stats:       config.serverStats,
		observer:    config.serverObserver,
		maxRequests: config.maxRequests,
	}
	if config.loadFn != nil {
		result.loadFn = config.loadFn
	} else {
		result.loadFn = result.defaultLoadFn
	}
	return result
}

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
	loadFn                  func() uint
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

func (s *rocketServer) acceptor(_ context.Context, setup payload.SetupPayload, sendingSocket rsocket.CloseableRSocket) (rsocket.RSocket, error) {
	if err := rocket.CheckRequestSetupMetadata8(setup); err != nil {
		return nil, err
	}
	serverMetadataPush, err := rocket.EncodeServerMetadataPush(s.zstdSupported)
	if err != nil {
		return nil, err
	}
	sendingSocket.MetadataPush(serverMetadataPush)
	socket := newRocketServerSocket(s)
	return rsocket.NewAbstractSocket(
		rsocket.MetadataPush(socket.metadataPush),
		rsocket.RequestResponse(socket.requestResponse),
		rsocket.FireAndForget(socket.fireAndForget),
		rsocket.RequestStream(socket.requestStream),
	), nil
}

// incrementActiveRequests increments the server-level active request counter and
// notifies the observer with the current total count across all sockets
func (s *rocketServer) incrementActiveRequests() {
	current := s.totalActiveRequestCount.Add(1)
	s.observer.ActiveRequests(int(current))
}

// decrementActiveRequests decrements the server-level active request counter and
// notifies the observer with the current total count across all sockets
func (s *rocketServer) decrementActiveRequests() {
	current := s.totalActiveRequestCount.Add(-1)
	s.observer.ActiveRequests(int(current))
}

// isOverloaded checks if adding one more request would exceed the configured maxRequests limit
// Returns true if the server should reject new requests due to high load
//
// TODO: align with C++ implementation
func (s *rocketServer) isOverloaded() bool {
	// If maxRequests is 0 (default), overload protection is disabled
	if s.maxRequests == 0 {
		return false
	}
	countWithNewRequest := s.totalActiveRequestCount.Load() + 1
	return countWithNewRequest > s.maxRequests
}

// This counter is what powers client side load balancing.
// loadFn is a function that reports system load.  It must report the
// server load as an unsigned integer.  Higher numbers mean the server
// is more loaded.  Clients choose the servers that report the lowest
// load.
// NOTE: if you run multiple servers with different capacities, you
// should ensure your load numbers are comparable and account for this
// (i.e. divide by NumCPU)
// NOTE: loadFn is called on every single response.  it should be fast.
func (s *rocketServer) defaultLoadFn() uint {
	working := s.totalActiveRequestCount.Load()
	denominator := float64(runtime.NumCPU())
	return uint(1000. * float64(working) / denominator)
}

type rocketServerSocket struct {
	*rocketServer

	// InteractionID to interaction processor map
	interactions      map[int64]Processor
	interactionsMutex sync.Mutex
}

func newRocketServerSocket(
	server *rocketServer,
) *rocketServerSocket {
	return &rocketServerSocket{
		rocketServer: server,
		interactions: make(map[int64]Processor),
	}
}

func (s *rocketServerSocket) metadataPush(msg payload.Payload) {
	// TODO: this clone helps prevent a race-condition where the payload gets
	// released by the underlying rsocket layer before we are done with it.
	msg = payload.Clone(msg)

	metadata := &rpcmetadata.ClientPushMetadata{}
	err := rocket.DecodePayloadMetadata(msg, metadata)
	if err != nil {
		s.log("unable to decode ClientPushMetadata: %v", err)
		return
	}

	if metadata.InteractionTerminate != nil {
		interactionID := metadata.InteractionTerminate.InteractionId
		s.interactionsMutex.Lock()
		interaction, ok := s.interactions[interactionID]
		delete(s.interactions, interactionID)
		s.interactionsMutex.Unlock()
		s.log("receive interaction terminate signal for ID %d", interactionID)
		if ok {
			if terminable, isTerminable := interaction.(types.Terminable); isTerminable {
				terminable.OnTermination()
			}
		}
	} else if metadata.StreamHeadersPush != nil {
		s.log("unsupported StreamHeadersPush metadata type")
	} else if metadata.TransportMetadataPush != nil {
		// s.log("unsupported TransportMetadataPush metadata type")
	}
}

func (s *rocketServerSocket) requestResponse(msg payload.Payload) mono.Mono {
	// TODO: this clone helps prevent a race-condition where the payload gets
	// released by the underlying rsocket layer before we are done with it.
	msg = payload.Clone(msg)

	requestReceivedTime := time.Now()

	metadata := rpcmetadata.NewRequestRpcMetadata()
	err := rocket.DecodePayloadMetadata(msg, metadata)
	if err != nil {
		// Notify observer that connection was dropped and task killed due to malformed rocket payload
		s.observer.ConnDropped()
		s.observer.TaskKilled()
		return mono.Error(err)
	}
	rpcFuncName := metadata.GetName()
	protocol, err := newProtocolBufferFromRequest(msg.Data(), metadata, s.observer)
	if err != nil {
		// Notify observer that connection was dropped and task killed due to protocol buffer creation error
		s.observer.ConnDropped()
		s.observer.TaskKilled()
		return mono.Error(err)
	}

	// Notify observer that request was received
	s.observer.ReceivedRequest()
	s.observer.ReceivedRequestForFunction(rpcFuncName)

	if s.isOverloaded() {
		// Track connection drops and server overload events when rejecting requests
		s.observer.ConnDropped()
		s.observer.ServerOverloaded()
		return mono.Error(loadSheddingError)
	}

	workItem := func(ctx context.Context) (payload.Payload, error) {
		// Increment active requests when processing actually starts
		s.incrementActiveRequests()
		defer s.decrementActiveRequests()

		// Track process delay from request received to processing start
		processStartTime := time.Now()
		processDelay := processStartTime.Sub(requestReceivedTime)
		s.observer.ProcessDelay(processDelay)

		processor := s.proc
		if metadata.InteractionCreate != nil {
			ctx = types.WithInteractionCreateContext(ctx)
		} else if metadata.InteractionId != nil {
			s.interactionsMutex.Lock()
			interactionProcessor, ok := s.interactions[*metadata.InteractionId]
			s.interactionsMutex.Unlock()
			if !ok {
				return nil, fmt.Errorf("unknown interaction id: %d", *metadata.InteractionId)
			}
			processor = interactionProcessor
		}

		// Track actual handler execution time
		appException, err := process(ctx, processor, protocol, s.pstats, s.observer)
		if err != nil {
			// Notify observer that connection was dropped due to unparseable message begin
			s.observer.ConnDropped()
			return nil, err
		}

		if metadata.InteractionCreate != nil && appException == nil {
			proc := types.GetInteractionCreateProcessor(ctx).(Processor)
			if proc == nil {
				return nil, fmt.Errorf("handler returned nil interaction processor: %d", metadata.InteractionCreate.InteractionId)
			}
			s.interactionsMutex.Lock()
			s.interactions[metadata.InteractionCreate.InteractionId] = proc
			s.interactionsMutex.Unlock()
		}

		protocol.setRequestHeader(LoadHeaderKey, fmt.Sprintf("%d", s.loadFn()))

		responseCompressionAlgo := rocket.CompressionAlgorithmFromCompressionConfig(metadata.GetCompressionConfig())
		var payload payload.Payload
		if appException != nil {
			payload, err = rocket.EncodeResponseApplicationErrorPayload(
				appException,
				protocol.getRequestHeaders(),
				responseCompressionAlgo,
			)
		} else {
			payload, err = rocket.EncodeResponsePayload(
				protocol.getRequestHeaders(),
				responseCompressionAlgo,
				protocol.Bytes(),
			)
		}

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

	metadata := rpcmetadata.NewRequestRpcMetadata()
	err := rocket.DecodePayloadMetadata(msg, metadata)
	if err != nil {
		// Notify observer that connection was dropped and task killed due to malformed rocket payload
		s.observer.ConnDropped()
		s.observer.TaskKilled()
		s.log("rocketServer fireAndForget decode request payload error: %v", err)
		return
	}
	rpcFuncName := metadata.GetName()
	protocol, err := newProtocolBufferFromRequest(msg.Data(), metadata, s.observer)
	if err != nil {
		// Notify observer that connection was dropped and task killed due to protocol buffer creation error
		s.observer.ConnDropped()
		s.observer.TaskKilled()
		s.log("rocketServer fireAndForget error creating protocol: %v", err)
		return
	}

	// Notify observer that request was received
	s.observer.ReceivedRequest()
	s.observer.ReceivedRequestForFunction(rpcFuncName)

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

	if _, err := process(context.Background(), s.proc, protocol, s.pstats, s.observer); err != nil {
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
	// TODO: this clone helps prevent a race-condition where the payload gets
	// released by the underlying rsocket layer before we are done with it.
	msg = payload.Clone(msg)

	metadata := rpcmetadata.NewRequestRpcMetadata()
	err := rocket.DecodePayloadMetadata(msg, metadata)
	if err != nil {
		s.observer.ConnDropped()
		s.observer.TaskKilled()
		return flux.Error(err)
	}
	rpcFuncName := metadata.GetName()
	protocol, err := newProtocolBufferFromRequest(msg.Data(), metadata, s.observer)
	if err != nil {
		s.observer.ConnDropped()
		s.observer.TaskKilled()
		return flux.Error(err)
	}

	s.observer.ReceivedRequest()
	s.observer.ReceivedRequestForFunction(rpcFuncName)

	pfunc, exists := s.proc.ProcessorFunctionMap()[rpcFuncName]
	if !exists {
		return flux.Error(fmt.Errorf("no such function: %q", rpcFuncName))
	}

	type ProcessorFunctionStream interface {
		RunStreamContext(
			ctx context.Context,
			args ReadableStruct,
			onFirstResponse func(WritableStruct),
			onStreamNext func(WritableStruct),
			onStreamComplete func(),
		)
	}
	pfuncStream, ok := pfunc.(ProcessorFunctionStream)
	if !ok {
		return flux.Error(fmt.Errorf("not a streaming function: %q", rpcFuncName))
	}

	argStruct := pfunc.NewReqArgs()
	if err := argStruct.Read(protocol); err != nil {
		return flux.Error(err)
	}
	if err := protocol.ReadMessageEnd(); err != nil {
		return flux.Error(err)
	}

	return flux.Create(
		func(ctx context.Context, sink flux.Sink) {
			protoID := types.ProtocolID(metadata.GetProtocol())
			responseCompressionAlgo := rocket.CompressionAlgorithmFromCompressionConfig(metadata.GetCompressionConfig())

			onFirstResponse := func(respStruct WritableStruct) {
				protocol, err := newProtocolBuffer(protoID, nil)
				if err != nil {
					s.log("rocketServer requestStream newProtocolBuffer error: %v", err)
					return
				}
				err = sendWritableStruct(protocol, rpcFuncName, types.REPLY, 0, respStruct)
				if err != nil {
					s.log("rocketServer requestStream sendWritableStruct error: %v", err)
					return
				}
				var payload payload.Payload
				appException, isAppException := respStruct.(*types.ApplicationException)
				if isAppException {
					payload, err = rocket.EncodeResponseApplicationErrorPayload(
						appException,
						protocol.getRequestHeaders(),
						responseCompressionAlgo,
					)
				} else {
					payload, err = rocket.EncodeResponsePayload(
						protocol.getRequestHeaders(),
						responseCompressionAlgo,
						protocol.Bytes(),
					)
				}
				if err != nil {
					s.log("rocketServer requestStream EncodeResponsePayload error: %v", err)
					return
				}
				sink.Next(payload)
			}
			onStreamNext := func(streamStruct WritableStruct) {
				protocol, err := newProtocolBuffer(protoID, nil)
				if err != nil {
					s.log("rocketServer requestStream newProtocolBuffer error: %v", err)
					return
				}
				err = sendWritableStruct(protocol, rpcFuncName, types.REPLY, 0, streamStruct)
				if err != nil {
					s.log("rocketServer requestStream sendWritableStruct error: %v", err)
					return
				}

				dataBytes := protocol.Bytes()
				payloadMetadata := rpcmetadata.NewPayloadMetadata()
				var exceptionMetadataBase *rpcmetadata.PayloadExceptionMetadataBase
				if appEx, ok := streamStruct.(*types.ApplicationException); ok {
					exceptionMetadataBase = rocket.NewPayloadExceptionMetadataBase(
						"ApplicationException",
						appEx.Error(),
						rocket.RocketExceptionAppUnknown,
						rpcmetadata.ErrorKind_UNSPECIFIED,
						rpcmetadata.ErrorBlame_UNSPECIFIED,
						rpcmetadata.ErrorSafety_UNSPECIFIED,
					)
					// Response should be empty to adhere to spec
					dataBytes = nil
				} else if streamResult, ok := streamStruct.(types.WritableResult); ok && streamResult.Exception() != nil {
					// TODO: implement support for getting this info from the underlying exception type
					declaredErr := streamResult.Exception()
					exType := fmt.Sprintf("%T", declaredErr)
					lastDotIndex := strings.LastIndex(exType, ".")
					if lastDotIndex != -1 && lastDotIndex < len(exType)-1 {
						exType = exType[lastDotIndex+1:]
					}
					exceptionMetadataBase = rocket.NewPayloadExceptionMetadataBase(
						exType,
						declaredErr.Error(),
						rocket.RocketExceptionDeclared,
						rpcmetadata.ErrorKind_UNSPECIFIED,
						rpcmetadata.ErrorBlame_UNSPECIFIED,
						rpcmetadata.ErrorSafety_UNSPECIFIED,
					)
				}

				if exceptionMetadataBase != nil {
					payloadMetadata.SetExceptionMetadata(exceptionMetadataBase)
				} else {
					responseMetadata := rpcmetadata.NewPayloadResponseMetadata()
					payloadMetadata.SetResponseMetadata(responseMetadata)
				}

				metadata := rpcmetadata.NewStreamPayloadMetadata().
					SetOtherMetadata(protocol.getRequestHeaders()).
					SetCompression(&responseCompressionAlgo).
					SetPayloadMetadata(payloadMetadata)

				payload, err := rocket.EncodePayloadMetadataAndData(metadata, dataBytes, responseCompressionAlgo)
				if err != nil {
					s.log("rocketServer requestStream EncodeStreamPayload error: %v", err)
					return
				}
				sink.Next(payload)
			}
			onStreamComplete := func() {
				sink.Complete()
			}
			pfuncStream.RunStreamContext(ctx, argStruct, onFirstResponse, onStreamNext, onStreamComplete)
		},
	)
}

func newProtocolBufferFromRequest(payloadDataBytes []byte, metadata *rpcmetadata.RequestRpcMetadata, observer ServerObserver) (*protocolBuffer, error) {
	rpcFuncName := metadata.GetName()
	dataBytes, err := rocket.MaybeDecompress(payloadDataBytes, metadata.GetCompression())
	if err != nil {
		return nil, fmt.Errorf("payload data bytes decompression failed: %w", err)
	}

	protoID := types.ProtocolID(metadata.GetProtocol())
	headersMap := rocket.GetRequestRpcMetadataHeaders(metadata)
	messageType := types.CALL
	if metadata.GetKind() == rpcmetadata.RpcKind_SINGLE_REQUEST_NO_RESPONSE {
		messageType = types.ONEWAY
	}

	protocol, err := newProtocolBuffer(protoID, dataBytes)
	if err != nil {
		return nil, err
	}
	protocol.setResponseHeaders(headersMap)
	if err := protocol.WriteMessageBegin(rpcFuncName, messageType, 0); err != nil {
		return nil, err
	}

	return protocol, nil
}
