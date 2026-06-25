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
	"iter"
	"math"
	"net"
	"runtime"
	"slices"
	"strconv"
	"sync"
	"sync/atomic"
	"time"

	"github.com/jjeffcaii/reactor-go/scheduler"
	rsocket "github.com/rsocket/rsocket-go"
	"github.com/rsocket/rsocket-go/core/transport"
	"github.com/rsocket/rsocket-go/payload"
	"github.com/rsocket/rsocket-go/rx/flux"
	"github.com/rsocket/rsocket-go/rx/mono"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/format"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/rocket"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
)

type processorFunctionStream interface {
	RunStreamContext(
		ctx context.Context,
		args ReadableStruct,
		onFirstResponse func(WritableStruct),
		onStreamNext func(WritableStruct),
		onStreamComplete func(),
	)
}

type processorFunctionSink interface {
	RunSinkContext(
		ctx context.Context,
		reqStruct ReadableStruct,
		onFirstResponse func(WritableStruct),
		onFinalResponse func(WritableStruct),
		onSinkError func(error),
		sinkSeq iter.Seq2[ReadableStruct, error],
	)
	NewSinkElem() ReadableResult
}

type processorFunctionBiDi interface {
	RunBiDiContext(
		ctx context.Context,
		reqStruct ReadableStruct,
		onFirstResponse func(WritableStruct),
		onStreamNext func(WritableStruct),
		onStreamComplete func(),
		sinkSeq iter.Seq2[ReadableStruct, error],
	)
	NewSinkElem() ReadableResult
}

var loadSheddingError = NewApplicationException(
	LOADSHEDDING,
	"load shedding due to max request limit",
)

var taskExpiredError = NewApplicationException(
	types.TIMEOUT,
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
	result := &server{
		proc:          proc,
		listener:      listener,
		transportID:   actualTransportID,
		zstdSupported: true,
		log:           config.log,
		connContext:   config.connContext,

		numWorkers: config.numWorkers,

		observer:     config.serverObserver,
		maxRequests:  config.maxRequests,
		interceptors: config.interceptors,
	}
	if config.loadFn != nil {
		result.loadFn = config.loadFn
	} else {
		result.loadFn = result.defaultLoadFn
	}
	return result
}

type server struct {
	proc          Processor
	listener      net.Listener
	transportID   TransportID
	zstdSupported bool
	log           func(format string, args ...any)
	connContext   ConnContextFunc

	numWorkers int

	observer                ServerObserver
	maxRequests             int64
	totalActiveRequestCount atomic.Int64
	loadFn                  func() uint32
	interceptors            []ServiceInterceptor
}

func (s *server) ServeContext(ctx context.Context) error {
	// TODO: support graceful shutdown and track with thrift.task_killed

	// Notify interceptors that the server is starting up, before accepting any
	// connections, so they can precompute schema-dependent state.
	err := s.runOnStartServingInterceptors()
	if err != nil {
		return err
	}

	transporter := func(context.Context) (transport.ServerTransport, error) {
		return newRocketServerTransport(
			s.listener,
			s.connContext,
			s.proc,
			s.transportID,
			s.log,
			s.observer,
		), nil
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

func (s *server) acceptor(ctx context.Context, setup payload.SetupPayload, sendingSocket rsocket.CloseableRSocket) (rsocket.RSocket, error) {
	if err := rocket.CheckRequestSetupMetadata8(setup); err != nil {
		return nil, err
	}
	serverMetadataPush, err := rocket.EncodeServerMetadataPush(s.zstdSupported)
	if err != nil {
		return nil, err
	}
	sendingSocket.MetadataPush(serverMetadataPush)
	connInfo, _ := connInfoFromContext(ctx)
	socket := newRocketServerSocket(s, connInfo)
	return rsocket.NewAbstractSocket(
		rsocket.MetadataPush(socket.metadataPush),
		rsocket.RequestResponse(socket.requestResponse),
		rsocket.FireAndForget(socket.fireAndForget),
		rsocket.RequestStream(socket.requestStream),
		rsocket.RequestChannel(socket.requestChannel),
	), nil
}

// incrementActiveRequests increments the server-level active request counter and
// notifies the observer with the current total count across all sockets
func (s *server) incrementActiveRequests() {
	current := s.totalActiveRequestCount.Add(1)
	s.observer.ActiveRequests(int(current))
}

// decrementActiveRequests decrements the server-level active request counter and
// notifies the observer with the current total count across all sockets
func (s *server) decrementActiveRequests() {
	current := s.totalActiveRequestCount.Add(-1)
	s.observer.ActiveRequests(int(current))
}

// isOverloaded checks if adding one more request would exceed the configured maxRequests limit
// Returns true if the server should reject new requests due to high load
//
// TODO: align with C++ implementation
func (s *server) isOverloaded() bool {
	// If maxRequests is 0 (default), overload protection is disabled
	if s.maxRequests == 0 {
		return false
	}
	countWithNewRequest := s.totalActiveRequestCount.Load() + 1
	return countWithNewRequest > s.maxRequests
}

// getQueueTimeout returns the queue timeout duration from metadata, or max duration if not set
func getQueueTimeout(metadata *rpcmetadata.RequestRpcMetadata) time.Duration {
	if metadata.IsSetQueueTimeoutMs() {
		queueTimeoutMs := metadata.GetQueueTimeoutMs()
		if queueTimeoutMs > 0 {
			return time.Duration(queueTimeoutMs) * time.Millisecond
		}
	}
	return time.Duration(math.MaxInt64)
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
func (s *server) defaultLoadFn() uint32 {
	working := s.totalActiveRequestCount.Load()
	denominator := float64(runtime.NumCPU())
	return uint32(1000. * float64(working) / denominator)
}

// runOnStartServingInterceptors invokes the OnStartServing callback of every
// registered ServiceInterceptor exactly once, in forward (registration) order,
// before the server accepts any connections. It builds the InitParams from the
// server's processor metadata so interceptors can precompute schema-dependent
// state.
//
// Unlike the per-request callbacks, startup is fail-fast: the first error
// returned by any interceptor aborts startup and is returned to the caller, so
// remaining interceptors are not invoked.
func (s *server) runOnStartServingInterceptors() error {
	initParams := InitParams{
		ServiceMetadata: s.proc.GetThriftMetadata(),
	}
	for _, interceptor := range s.interceptors {
		err := interceptor.OnStartServing(initParams)
		if err != nil {
			return fmt.Errorf("service interceptor startup failed: %w", err)
		}
	}
	return nil
}

// runOnRequestInterceptors invokes every registered ServiceInterceptor's
// OnRequest in forward (registration) order, after the request is deserialized
// and before the handler runs.
//
// Each interceptor returns the context to use for the rest of the request;
// interceptors carry per-request state by storing it in that context and reading
// it back in OnResponse. To protect the chain, the returned context is accepted
// only if it is non-nil and still carries an unexported sentinel, proving it was
// derived from the one we passed in; otherwise it is discarded with a warning
// and the previous context carried forward.
//
// All interceptors always run, even if an earlier one errors; the first error is
// returned and signals that the handler must not run.
//
// userConnState is currently always nil: connection-scoped state is not yet
// supported.
func (s *server) runOnRequestInterceptors(ctx context.Context, req ReadableStruct) (context.Context, error) {
	if len(s.interceptors) == 0 {
		return ctx, nil
	}
	// interceptorContextSentinelKey marks the context we hand to OnRequest. A
	// returned context is accepted only if it still carries this sentinel,
	// proving it was derived from ours and not replaced with nil or a fresh
	// context that would drop request-scoped values.
	type interceptorContextSentinelKey struct{}
	var firstErr error
	// Tag the context so we can detect a context not derived from this one.
	ctx = context.WithValue(ctx, interceptorContextSentinelKey{}, struct{}{})
	for _, interceptor := range s.interceptors {
		ctxPrime, err := interceptor.OnRequest(ctx, req, nil /* userConnState */)
		if err != nil && firstErr == nil {
			firstErr = err
		}
		// Discard a nil context, or one missing our sentinel, and keep the
		// previous one.
		if ctxPrime == nil || ctxPrime.Value(interceptorContextSentinelKey{}) == nil {
			s.log("thrift: ServiceInterceptor %T OnRequest returned an invalid context; discarding it", interceptor)
			continue
		}
		ctx = ctxPrime
	}
	return ctx, firstErr
}

// runOnResponseInterceptors invokes the OnResponse callback of every registered
// ServiceInterceptor in reverse order relative to registration, so that the
// first interceptor to observe OnRequest is the last to observe OnResponse
// (matching the C++ ServiceInterceptor ordering contract). It is meant to be
// called by the server's RPC handling paths before the outgoing response is
// serialized.
//
// The ctx passed in is the one returned by runOnRequestInterceptors, so any
// per-request state an interceptor stored in the context during OnRequest can be
// read back from ctx in OnResponse.
//
// All interceptors are always invoked, even if one returns an error. When
// multiple interceptors return errors, the last one encountered is returned.
// Because iteration is in reverse, that is the error from the earliest-registered
// interceptor, matching the C++ behavior where an OnResponse exception overwrites
// any currently-active exception.
func (s *server) runOnResponseInterceptors(ctx context.Context, resp WritableStruct) error {
	var respErr error
	for _, interceptor := range slices.Backward(s.interceptors) {
		err := interceptor.OnResponse(ctx, resp)
		if err != nil {
			respErr = err
		}
	}
	return respErr
}

type rocketServerSocket struct {
	*server

	connInfo ConnInfo

	// InteractionID to interaction processor map
	interactions      map[int64]Processor
	interactionsMutex sync.Mutex
}

func newRocketServerSocket(
	server *server,
	connInfo ConnInfo,
) *rocketServerSocket {
	return &rocketServerSocket{
		server:       server,
		connInfo:     connInfo,
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
	requestReceivedTime := time.Now()

	metadata, pfunc, argStruct, reqCtx, err := s.preprocessRequest(msg)
	if err != nil {
		return mono.Error(err)
	}
	rpcFuncName := metadata.GetName()

	workItem := func(ctx context.Context) (payload.Payload, error) {
		// Increment active requests when processing actually starts
		s.incrementActiveRequests()
		defer s.decrementActiveRequests()

		// Track process delay from request received to processing start
		processStartTime := time.Now()
		processDelay := processStartTime.Sub(requestReceivedTime)
		s.observer.ProcessDelay(processDelay)

		queueTimeout := getQueueTimeout(metadata)
		if processDelay > queueTimeout {
			s.observer.TaskTimeout()
			return nil, taskExpiredError
		}

		if metadata.InteractionCreate != nil {
			ctx = types.WithInteractionCreateContext(ctx)
		}
		ctx = WithRequestContext(ctx, reqCtx)

		// Run OnRequest interceptors before the handler.
		ctx, reqIntErr := s.runOnRequestInterceptors(ctx, argStruct)

		var result WritableResult
		var resErr error
		func() {
			// Skip handler if onRequest interceptors returned an error
			if reqIntErr != nil {
				resErr = reqIntErr
				return
			}

			defer func() {
				if r := recover(); r != nil {
					s.observer.ProcessorPanic()
					panic(r)
				}
			}()
			pfuncStartTime := time.Now()
			result, resErr = pfunc.RunContext(ctx, argStruct)
			s.observer.TimeProcessUsForFunction(rpcFuncName, time.Since(pfuncStartTime))
		}()

		// Select the response struct: an undeclared error (from OnRequest or the
		// handler) becomes an ApplicationException; otherwise the handler result
		// (which may carry a declared exception).
		var respStruct WritableStruct = result
		if resErr != nil {
			respStruct = maybeWrapApplicationException(resErr)
		}

		// Only register the interaction if the factory call succeeded. A failed
		// factory (undeclared error or declared exception) must not create an
		// interaction; the client treats it as a creation failure
		// (INTERACTION_CONSTRUCTOR_ERROR) and recreates.
		responseHasException := (resErr != nil || result.Exception() != nil)

		if metadata.InteractionCreate != nil && !responseHasException {
			proc := types.GetInteractionCreateProcessor(ctx).(Processor)
			if proc == nil {
				return nil, fmt.Errorf("handler returned nil interaction processor: %d", metadata.InteractionCreate.InteractionId)
			}
			s.interactionsMutex.Lock()
			s.interactions[metadata.InteractionCreate.InteractionId] = proc
			s.interactionsMutex.Unlock()
		}

		payload, err := s.makeResponsePayload(metadata, respStruct, true /* isFirstResponse */)
		if err != nil {
			s.observer.ConnDropped()
			return nil, err
		}

		// Track actual handler execution time
		s.observer.ProcessTime(time.Since(processStartTime))
		s.observer.SentReply()

		return payload, err
	}
	return mono.FromFunc(workItem)
}

func (s *rocketServerSocket) fireAndForget(msg payload.Payload) {
	requestReceivedTime := time.Now()

	metadata, pfunc, argStruct, reqCtx, err := s.preprocessRequest(msg)
	if err != nil {
		s.log("server fireAndForget preprocess error: %v", err)
		return
	}
	rpcFuncName := metadata.GetName()

	// Increment active requests when processing starts
	s.incrementActiveRequests()
	defer s.decrementActiveRequests()

	// Track process delay from request received to processing start
	processStartTime := time.Now()
	processDelay := processStartTime.Sub(requestReceivedTime)
	s.observer.ProcessDelay(processDelay)

	queueTimeout := getQueueTimeout(metadata)
	if processDelay > queueTimeout {
		s.observer.TaskTimeout()
		s.log("server fireAndForget: dropping request due to queue timeout")
		return
	}

	// Oneway requests have no per-request scheduler context; derive one from the
	// background context so the handler can access the RequestContext.
	ctx := WithRequestContext(context.Background(), reqCtx)

	// Run OnRequest interceptors before the handler.
	ctx, reqIntErr := s.runOnRequestInterceptors(ctx, argStruct)

	var resErr error
	func() {
		// Skip handler if onRequest interceptors returned an error
		if reqIntErr != nil {
			resErr = reqIntErr
			return
		}

		defer func() {
			if r := recover(); r != nil {
				s.observer.ProcessorPanic()
				panic(r)
			}
		}()
		pfuncStartTime := time.Now()
		_, resErr = pfunc.RunContext(ctx, argStruct)
		s.observer.TimeProcessUsForFunction(rpcFuncName, time.Since(pfuncStartTime))
	}()

	if resErr != nil {
		s.observer.ConnDropped()
		s.log("server fireAndForget error: %v", resErr)
	}

	// Track actual handler execution time
	s.observer.ProcessTime(time.Since(processStartTime))
}

func (s *rocketServerSocket) requestStream(msg payload.Payload) flux.Flux {
	metadata, pfunc, argStruct, reqCtx, err := s.preprocessRequest(msg)
	if err != nil {
		return flux.Error(err)
	}

	rpcFuncName := metadata.GetName()

	pfuncStream, ok := pfunc.(processorFunctionStream)
	if !ok {
		return flux.Error(fmt.Errorf("not a streaming function: %q", rpcFuncName))
	}

	return flux.Create(
		func(ctx context.Context, sink flux.Sink) {
			ctx = WithRequestContext(ctx, reqCtx)

			onFirstResponse := func(respStruct WritableStruct) {
				respPayload, err := s.makeResponsePayload(metadata, respStruct, true /* isFirstResponse */)
				if err != nil {
					s.log("server requestStream makeResponsePayload error: %v", err)
					return
				}
				sink.Next(respPayload)
			}
			onStreamNext := func(streamStruct WritableStruct) {
				streamPayload, err := s.makeResponsePayload(metadata, streamStruct, false /* isFirstResponse */)
				if err != nil {
					s.log("server requestStream makeResponsePayload error: %v", err)
					return
				}
				sink.Next(streamPayload)
			}
			onStreamComplete := func() {
				sink.Complete()
			}

			// Run OnRequest interceptors before the handler.
			ctx, reqIntErr := s.runOnRequestInterceptors(ctx, argStruct)
			if reqIntErr != nil {
				onFirstResponse(maybeWrapApplicationException(reqIntErr))
				onStreamComplete()
				return
			}

			pfuncStream.RunStreamContext(ctx, argStruct, onFirstResponse, onStreamNext, onStreamComplete)
		},
	)
}

func (s *rocketServerSocket) requestChannel(request payload.Payload, requests flux.Flux) flux.Flux {
	metadata, pfunc, argStruct, reqCtx, err := s.preprocessRequest(request)
	if err != nil {
		return flux.Error(err)
	}

	rpcFuncName := metadata.GetName()
	if pfuncSink, ok := pfunc.(processorFunctionSink); ok {
		return s.requestChannelSink(metadata, reqCtx, argStruct, pfuncSink, requests)
	}
	if pfuncBiDi, ok := pfunc.(processorFunctionBiDi); ok {
		return s.requestChannelBiDi(metadata, reqCtx, argStruct, pfuncBiDi, requests)
	}
	return flux.Error(fmt.Errorf("not a sink or bidi function: %q", rpcFuncName))
}

func (s *rocketServerSocket) requestChannelSink(
	metadata *rpcmetadata.RequestRpcMetadata,
	reqCtx *RequestContext,
	argStruct ReadableStruct,
	pfuncSink processorFunctionSink,
	requests flux.Flux,
) flux.Flux {
	sinkElemChan := make(chan ReadableStruct, types.DefaultStreamBufferSize)
	sinkErrChan := make(chan error, 1)

	sinkSeq := func(yield func(ReadableStruct, error) bool) {
		// Drain all buffered sink elements first, then surface any error.
		// DoOnComplete/DoOnError closes sinkElemChan after the last
		// DoOnNext, so this loop terminates after the queue is drained.
		for elem := range sinkElemChan {
			if !yield(elem, nil) {
				return
			}
		}
		select {
		case err := <-sinkErrChan:
			yield(nil, err)
		default:
		}
	}

	return flux.Create(func(ctx context.Context, sink flux.Sink) {
		ctx = WithRequestContext(ctx, reqCtx)
		firstResponseQueued := make(chan struct{})

		onFirstResponse := func(respStruct WritableStruct) {
			respPayload, err := s.makeResponsePayload(metadata, respStruct, true /* isFirstResponse */)
			if err != nil {
				s.log("server requestChannel makeResponsePayload error: %v", err)
				return
			}
			sink.Next(respPayload)
			close(firstResponseQueued)
		}

		onFinalResponse := func(respStruct WritableStruct) {
			finalPayload, err := s.makeResponsePayload(metadata, respStruct, false /* isFirstResponse */)
			if err != nil {
				s.log("server requestChannel makeResponsePayload error: %v", err)
				return
			}
			sink.Next(rsocket.NewFinalPayload(finalPayload))
			sink.Complete()
		}

		onSinkError := sink.Error

		// Run OnRequest interceptors before the handler.
		ctx, reqIntErr := s.runOnRequestInterceptors(ctx, argStruct)
		if reqIntErr != nil {
			onFirstResponse(maybeWrapApplicationException(reqIntErr))
			sink.Complete()
			return
		}

		// Start the sink processor in a goroutine.
		// It will call onFirstResponse which queues FirstResponse to the sink.
		// Then start the requests subscription after FirstResponse is queued.
		go pfuncSink.RunSinkContext(ctx, argStruct, onFirstResponse, onFinalResponse, onSinkError, sinkSeq)

		// Start a separate goroutine that waits for FirstResponse to be queued,
		// then subscribes to requests (triggers RequestN to client).
		// This matches the C++ implementation where bridge.start() is scheduled
		// AFTER onFirstResponse is called synchronously.

		// Wait for FirstResponse to be queued to the sink
		<-firstResponseQueued

		// Now subscribe to sink elements from the requests flux.
		// This triggers RequestN to the client for more payloads.
		requests.
			DoOnNext(func(msg payload.Payload) error {
				msg = payload.Clone(msg)

				// All items from requests flux are sink elements
				sinkPayloadMetadata := rpcmetadata.NewStreamPayloadMetadata()
				if err := rocket.DecodePayloadMetadata(msg, sinkPayloadMetadata); err != nil {
					s.log("server requestChannel decode sink element metadata error: %v", err)
					return nil
				}

				if sinkPayloadMetadata.IsSetPayloadMetadata() {
					if sinkPayloadMetadata.PayloadMetadata.IsSetExceptionMetadata() {
						exceptionMetadata := sinkPayloadMetadata.PayloadMetadata.ExceptionMetadata
						if exceptionMetadata.IsSetMetadata() {
							if exceptionMetadata.Metadata.IsSetAppUnknownException() {
								appEx := types.NewApplicationException(
									types.UNKNOWN_APPLICATION_EXCEPTION,
									exceptionMetadata.GetWhatUTF8(),
								)
								sinkErrChan <- appEx
								return nil
							}
						}
					}
				}

				compression := rpcmetadata.CompressionAlgorithm_NONE
				if sinkPayloadMetadata.Compression != nil {
					compression = *sinkPayloadMetadata.Compression
				}
				dataBytes, err := rocket.MaybeDecompress(msg.Data(), compression)
				if err != nil {
					s.log("server requestChannel decompress sink element error: %v", err)
					return nil
				}

				sinkElemStruct := pfuncSink.NewSinkElem()
				switch metadata.GetProtocol() {
				case rpcmetadata.ProtocolId_BINARY:
					err = format.DecodeBinary(dataBytes, sinkElemStruct)
				case rpcmetadata.ProtocolId_COMPACT:
					err = format.DecodeCompact(dataBytes, sinkElemStruct)
				default:
					err = types.NewProtocolException(fmt.Errorf("unknown protocol id: %d", metadata.GetProtocol()))
				}
				if err != nil {
					s.log("server requestChannel read sink element error: %v", err)
					return nil
				}
				if sinkEx := sinkElemStruct.Exception(); sinkEx != nil {
					sinkErrChan <- sinkEx
					return nil
				}

				sinkElemChan <- sinkElemStruct
				return nil
			}).
			DoOnError(func(err error) {
				sinkErrChan <- err
				close(sinkElemChan)
			}).
			DoOnComplete(func() {
				close(sinkElemChan)
			}).
			Subscribe(ctx)
	})
}

func (s *rocketServerSocket) requestChannelBiDi(
	metadata *rpcmetadata.RequestRpcMetadata,
	reqCtx *RequestContext,
	argStruct ReadableStruct,
	pfuncBiDi processorFunctionBiDi,
	requests flux.Flux,
) flux.Flux {
	sinkElemChan := make(chan ReadableStruct, types.DefaultStreamBufferSize)
	sinkErrChan := make(chan error, 1)

	sinkSeq := func(yield func(ReadableStruct, error) bool) {
		// Drain all buffered sink elements first, then surface any error.
		for elem := range sinkElemChan {
			if !yield(elem, nil) {
				return
			}
		}
		select {
		case err := <-sinkErrChan:
			yield(nil, err)
		default:
		}
	}

	return flux.Create(func(ctx context.Context, sink flux.Sink) {
		ctx = WithRequestContext(ctx, reqCtx)
		firstResponseQueued := make(chan struct{})

		onFirstResponse := func(respStruct WritableStruct) {
			respPayload, err := s.makeResponsePayload(metadata, respStruct, true /* isFirstResponse */)
			if err != nil {
				s.log("server requestChannel makeResponsePayload error: %v", err)
				return
			}
			sink.Next(respPayload)
			close(firstResponseQueued)
		}

		onStreamNext := func(respStruct WritableStruct) {
			streamPayload, err := s.makeResponsePayload(metadata, respStruct, false /* isFirstResponse */)
			if err != nil {
				s.log("server requestChannel makeResponsePayload error: %v", err)
				return
			}
			sink.Next(streamPayload)
		}

		onStreamComplete := func() {
			sink.Complete()
		}

		// Run OnRequest interceptors before the handler.
		ctx, reqIntErr := s.runOnRequestInterceptors(ctx, argStruct)
		if reqIntErr != nil {
			onFirstResponse(maybeWrapApplicationException(reqIntErr))
			onStreamComplete()
			return
		}

		go pfuncBiDi.RunBiDiContext(ctx, argStruct, onFirstResponse, onStreamNext, onStreamComplete, sinkSeq)

		<-firstResponseQueued

		requests.
			DoOnNext(func(msg payload.Payload) error {
				msg = payload.Clone(msg)

				sinkPayloadMetadata := rpcmetadata.NewStreamPayloadMetadata()
				if err := rocket.DecodePayloadMetadata(msg, sinkPayloadMetadata); err != nil {
					s.log("server requestChannel decode sink element metadata error: %v", err)
					return nil
				}

				if sinkPayloadMetadata.IsSetPayloadMetadata() {
					if sinkPayloadMetadata.PayloadMetadata.IsSetExceptionMetadata() {
						exceptionMetadata := sinkPayloadMetadata.PayloadMetadata.ExceptionMetadata
						if exceptionMetadata.IsSetMetadata() {
							if exceptionMetadata.Metadata.IsSetAppUnknownException() {
								appEx := types.NewApplicationException(
									types.UNKNOWN_APPLICATION_EXCEPTION,
									exceptionMetadata.GetWhatUTF8(),
								)
								sinkErrChan <- appEx
								return nil
							}
						}
					}
				}

				compression := rpcmetadata.CompressionAlgorithm_NONE
				if sinkPayloadMetadata.Compression != nil {
					compression = *sinkPayloadMetadata.Compression
				}
				dataBytes, err := rocket.MaybeDecompress(msg.Data(), compression)
				if err != nil {
					s.log("server requestChannel decompress sink element error: %v", err)
					return nil
				}

				sinkElemStruct := pfuncBiDi.NewSinkElem()
				switch metadata.GetProtocol() {
				case rpcmetadata.ProtocolId_BINARY:
					err = format.DecodeBinary(dataBytes, sinkElemStruct)
				case rpcmetadata.ProtocolId_COMPACT:
					err = format.DecodeCompact(dataBytes, sinkElemStruct)
				default:
					err = types.NewProtocolException(fmt.Errorf("unknown protocol id: %d", metadata.GetProtocol()))
				}
				if err != nil {
					s.log("server requestChannel read sink element error: %v", err)
					return nil
				}
				if sinkEx := sinkElemStruct.Exception(); sinkEx != nil {
					sinkErrChan <- sinkEx
					return nil
				}

				sinkElemChan <- sinkElemStruct
				return nil
			}).
			DoOnError(func(err error) {
				sinkErrChan <- err
				close(sinkElemChan)
			}).
			DoOnComplete(func() {
				close(sinkElemChan)
			}).
			Subscribe(ctx)
	})
}

// makeResponsePayload encodes a single response struct into a rocket payload.
// isFirstResponse selects the initial-response encoding (request-response style)
// over the subsequent stream-element encoding. It is shared by the streaming RPC
// paths (stream, sink, bidi).
func (s *rocketServerSocket) makeResponsePayload(
	metadata *rpcmetadata.RequestRpcMetadata,
	respStruct WritableStruct,
	isFirstResponse bool,
) (payload.Payload, error) {
	rpcFuncName := metadata.GetName()
	protoID := metadata.GetProtocol()
	compression := rocket.CompressionAlgorithmFromCompressionConfig(metadata.GetCompressionConfig())

	writeStartTime := time.Now()

	var dataBytes []byte
	var err error
	switch protoID {
	case rpcmetadata.ProtocolId_BINARY:
		dataBytes, err = format.EncodeBinary(respStruct)
	case rpcmetadata.ProtocolId_COMPACT:
		dataBytes, err = format.EncodeCompact(respStruct)
	default:
		return nil, types.NewProtocolException(fmt.Errorf("unknown protocol id: %d", protoID))
	}
	if err != nil {
		return nil, err
	}

	// Wrapped in a closure so time.Since is evaluated at return, not eagerly at
	// the defer statement (deferred-call arguments are evaluated immediately).
	defer func() {
		s.observer.TimeWriteUsForFunction(rpcFuncName, time.Since(writeStartTime))
	}()

	headers := map[string]string{}

	// Build exception metadata if applicable, reporting both undeclared and
	// declared exceptions to the observer.
	var exceptionMetadata *rpcmetadata.PayloadExceptionMetadataBase
	var exceptionErr error
	if appEx, ok := respStruct.(*types.ApplicationException); ok {
		exceptionMetadata = rocket.NewPayloadExceptionMetadataBaseV2(appEx)
		exceptionErr = appEx
		dataBytes = nil
		s.observer.UndeclaredExceptionForFunction(rpcFuncName)
	} else if streamResult, ok := respStruct.(types.WritableResult); ok && streamResult.Exception() != nil {
		declaredErr := streamResult.Exception()
		exceptionMetadata = rocket.NewPayloadExceptionMetadataBaseV2(declaredErr)
		exceptionErr = declaredErr
		s.observer.DeclaredExceptionForFunction(rpcFuncName)
	}

	if isFirstResponse {
		loadMetric := s.loadFn()
		loadMetricPtr := (*int64)(nil)
		if metadata.IsSetLoadMetric() {
			loadMetricPtr = Pointerize(int64(loadMetric))
		}
		headers[LoadHeaderKey] = strconv.FormatUint(uint64(loadMetric), 10)
		if exceptionErr != nil {
			headers["uex"] = errorType(exceptionErr)
			headers["uexw"] = exceptionErr.Error()
		}
		if appException, ok := respStruct.(*types.ApplicationException); ok {
			return rocket.EncodeResponseApplicationErrorPayload(
				appException,
				headers,
				compression,
				loadMetricPtr,
			)
		}
		return rocket.EncodeResponsePayload(
			headers,
			compression,
			loadMetricPtr,
			dataBytes,
		)
	}

	// Subsequent stream-element encoding.
	payloadMetadata := rpcmetadata.NewPayloadMetadata()
	if exceptionMetadata != nil {
		payloadMetadata.SetExceptionMetadata(exceptionMetadata)
	} else {
		payloadMetadata.SetResponseMetadata(rpcmetadata.NewPayloadResponseMetadata())
	}

	streamMetadata := rpcmetadata.NewStreamPayloadMetadata().
		SetOtherMetadata(headers).
		SetCompression(&compression).
		SetPayloadMetadata(payloadMetadata)

	return rocket.EncodePayloadMetadataAndData(streamMetadata, dataBytes, compression)
}

// preprocessRequest performs the shared per-request setup for the streaming RPC
// paths: it clones and decodes the rocket payload, builds the request protocol
// buffer, applies overload protection, resolves the processor function, and
// reads the request argument struct. Callers type-assert the returned
// ProcessorFunction to the concrete interaction kind they support (stream /
// sink / bidi) and wrap any returned error in the transport-appropriate form
// (e.g. flux.Error). It also returns a *RequestContext built from the request
// metadata and connection info; callers attach it to the request ctx.
func (s *rocketServerSocket) preprocessRequest(msg payload.Payload) (
	*rpcmetadata.RequestRpcMetadata,
	ProcessorFunction,
	ReadableStruct,
	*RequestContext,
	error,
) {
	// TODO: this clone helps prevent a race-condition where the payload gets
	// released by the underlying rsocket layer before we are done with it.
	msg = payload.Clone(msg)

	metadata := rpcmetadata.NewRequestRpcMetadata()
	err := rocket.DecodePayloadMetadata(msg, metadata)
	if err != nil {
		s.observer.ConnDropped()
		s.observer.TaskKilled()
		return nil, nil, nil, nil, err
	}

	rpcFuncName := metadata.GetName()
	s.observer.ReceivedRequestForFunction(rpcFuncName)

	if s.isOverloaded() {
		s.observer.ConnDropped()
		s.observer.ServerOverloaded()
		return nil, nil, nil, nil, loadSheddingError
	}

	readStartTime := time.Now()
	dataBytes, err := rocket.MaybeDecompress(msg.Data(), metadata.GetCompression())
	if err != nil {
		s.observer.ConnDropped()
		s.observer.TaskKilled()
		return nil, nil, nil, nil, fmt.Errorf("payload data bytes decompression failed: %w", err)
	}
	processor := s.proc
	if metadata.InteractionId != nil {
		s.interactionsMutex.Lock()
		interactionProcessor, ok := s.interactions[*metadata.InteractionId]
		s.interactionsMutex.Unlock()
		if !ok {
			return nil, nil, nil, nil, fmt.Errorf("unknown interaction id: %d", *metadata.InteractionId)
		}
		processor = interactionProcessor
	}

	pfunc, exists := processor.ProcessorFunctionMap()[rpcFuncName]
	if !exists {
		return nil, nil, nil, nil, fmt.Errorf("no such function: %q", rpcFuncName)
	}

	argStruct := pfunc.NewReqArgs()
	protoID := metadata.GetProtocol()
	switch protoID {
	case rpcmetadata.ProtocolId_BINARY:
		err = format.DecodeBinary(dataBytes, argStruct)
	case rpcmetadata.ProtocolId_COMPACT:
		err = format.DecodeCompact(dataBytes, argStruct)
	default:
		return nil, nil, nil, nil, types.NewProtocolException(fmt.Errorf("unknown protocol id: %d", protoID))
	}
	if err != nil {
		return nil, nil, nil, nil, err
	}
	s.observer.TimeReadUsForFunction(rpcFuncName, time.Since(readStartTime))

	reqHeaders := rocket.GetRequestRpcMetadataHeaders(metadata)
	reqCtx := &RequestContext{
		ServiceName: processor.FunctionServiceMap()[rpcFuncName],
		MethodName:  rpcFuncName,
		ConnInfo:    s.connInfo,
	}
	reqCtx.SetReadHeaders(reqHeaders)

	return metadata, pfunc, argStruct, reqCtx, nil
}
