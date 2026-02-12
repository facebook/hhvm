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
	"bytes"
	"context"
	"fmt"
	"iter"
	"math"
	"net"
	"time"

	"golang.org/x/sync/singleflight"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/format"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/rocket"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
	"github.com/jjeffcaii/reactor-go/scheduler"
	rsocket "github.com/rsocket/rsocket-go"
	"github.com/rsocket/rsocket-go/core/transport"
	"github.com/rsocket/rsocket-go/payload"
	"github.com/rsocket/rsocket-go/rx/flux"
)

// RSocketClient is a client that uses a rsocket library.
type RSocketClient interface {
	SendSetup(ctx context.Context) error
	FireAndForget(
		ctx context.Context,
		messageName string,
		headers map[string]string,
		dataBytes []byte,
	) error
	RequestResponse(
		ctx context.Context,
		messageName string,
		headers map[string]string,
		dataBytes []byte,
	) (map[string]string, []byte, error)
	RequestStream(
		ctx context.Context,
		messageName string,
		headers map[string]string,
		dataBytes []byte,
		newStreamElemFn func() ReadableResult,
	) (map[string]string, []byte, iter.Seq2[ReadableStruct, error], error)
	RequestChannel(
		ctx context.Context,
		messageName string,
		headers map[string]string,
		dataBytes []byte,
	) (map[string]string, []byte, func(sinkSeq iter.Seq2[WritableResult, error], finalResponse ReadableStruct) error, error)
	MetadataPush(
		ctx context.Context,
		metadata *rpcmetadata.ClientPushMetadata,
	) error
	Close() error
}

type rsocketClient struct {
	client rsocket.Client
	conn   net.Conn

	clientScheduler scheduler.Scheduler

	initGroup singleflight.Group

	useZstd bool

	protoID       rpcmetadata.ProtocolId
	thriftProtoID types.ProtocolID
}

func newRSocketClient(conn net.Conn, protoID rpcmetadata.ProtocolId, thriftProtoID types.ProtocolID) RSocketClient {
	return &rsocketClient{
		conn:            conn,
		clientScheduler: scheduler.NewElastic(math.MaxInt32),
		protoID:         protoID,
		thriftProtoID:   thriftProtoID,
	}
}

func (r *rsocketClient) SendSetup(_ context.Context) error {
	// Only a single instance of the code below is allowed to run at a time.
	// If multiple goroutines arrive at this point concurrently - they will
	// all wait here for a single instance of "setup" to complete.
	// If "setup" has already run (client != nil) - we return immediately.
	// If "setup" has already run but failed with an error (client == nil),
	// the code below will be retried by the next goroutine that arrives here.
	_, setupErr, _ := r.initGroup.Do("setup", func() (any, error) {
		if r.client != nil {
			return nil, nil
		}

		setupPayload, err := rocket.NewRequestSetupPayloadVersion8()
		if err != nil {
			return nil, err
		}
		// Very important to reset the deadline! Especially when using UpgradeToRocket.
		// We may have inherited this connection from Header protocol after an Upgrade.
		// Deadlines may be nearing expiration, if not reset - rsocket setup may fail.
		r.resetDeadline()
		// See T182939211. This copies the keep alives from Java Rocket.
		// KeepaliveLifetime = time.Duration(missedAcks = 1) * (ackTimeout = 3600000)
		clientBuilder := rsocket.Connect().
			KeepAlive(time.Millisecond*30000, time.Millisecond*3600000, 1).
			MetadataMimeType(rocket.RocketMetadataCompactMimeType).
			Scheduler(r.clientScheduler, r.clientScheduler).
			SetupPayload(setupPayload).
			OnClose(func(error) {})

		clientStarter := clientBuilder.Acceptor(
			func(_ context.Context, _ rsocket.RSocket) rsocket.RSocket {
				return rsocket.NewAbstractSocket(
					rsocket.MetadataPush(
						r.onServerMetadataPush,
					),
				)
			},
		)

		client, err := clientStarter.Transport(transporter(r.conn)).Start(context.Background())
		if err != nil {
			return nil, err
		}
		r.client = client
		return nil, nil
	})

	return setupErr
}

func (r *rsocketClient) onServerMetadataPush(pay payload.Payload) {
	metadata, err := rocket.DecodeServerMetadataPush(pay)
	if err != nil {
		panic(err)
	}
	if metadata.SetupResponse != nil {
		setupResponse := metadata.SetupResponse
		serverSupportsZstd := (setupResponse.ZstdSupported != nil && *setupResponse.ZstdSupported)
		// zstd is only supported if both the client and the server support it.
		r.useZstd = r.useZstd && serverSupportsZstd
	}
}

func transporter(conn net.Conn) func(_ context.Context) (*transport.Transport, error) {
	return func(_ context.Context) (*transport.Transport, error) {
		tconn := transport.NewTCPClientTransport(conn)
		tconn.SetLifetime(1 * time.Hour)
		return tconn, nil
	}
}

func (r *rsocketClient) resetDeadline() {
	// It is necessary to reset the deadline to 0.
	// The rsocket library only sets the deadline at connection start.
	// This means if you wait long enough, the connection will become useless.
	// Or something else is happening, but this is very necessary.
	r.conn.SetDeadline(time.Time{})
}

func (r *rsocketClient) RequestResponse(
	ctx context.Context,
	messageName string,
	headers map[string]string,
	dataBytes []byte,
) (map[string]string, []byte, error) {
	r.resetDeadline()
	request, err := rocket.EncodeRequestPayload(
		ctx,
		messageName,
		r.protoID,
		rpcmetadata.RpcKind_SINGLE_REQUEST_SINGLE_RESPONSE,
		headers,
		rpcmetadata.CompressionAlgorithm_NONE,
		dataBytes,
	)
	if err != nil {
		return nil, nil, err
	}
	mono := r.client.RequestResponse(request)
	val, err := mono.Block(ctx)
	if err != nil {
		return nil, nil, err
	}
	response, err := rocket.DecodeResponsePayload(val)
	if response != nil {
		return response.Headers(), response.Data(), err
	}
	return nil, nil, err
}

func (r *rsocketClient) FireAndForget(ctx context.Context, messageName string, headers map[string]string, dataBytes []byte) error {
	r.resetDeadline()
	request, err := rocket.EncodeRequestPayload(
		ctx,
		messageName,
		r.protoID,
		rpcmetadata.RpcKind_SINGLE_REQUEST_NO_RESPONSE,
		headers,
		rpcmetadata.CompressionAlgorithm_NONE,
		dataBytes,
	)
	if err != nil {
		return err
	}
	r.client.FireAndForget(request)
	return nil
}

func (r *rsocketClient) RequestStream(
	ctx context.Context,
	messageName string,
	headers map[string]string,
	dataBytes []byte,
	newStreamElemFn func() ReadableResult,
) (map[string]string, []byte, iter.Seq2[ReadableStruct, error], error) {
	r.resetDeadline()

	request, err := rocket.EncodeRequestPayload(
		ctx,
		messageName,
		r.protoID,
		rpcmetadata.RpcKind_SINGLE_REQUEST_STREAMING_RESPONSE,
		headers,
		rpcmetadata.CompressionAlgorithm_NONE,
		dataBytes,
	)
	if err != nil {
		return nil, nil, nil, err
	}

	flux := r.client.RequestStream(request)

	streamCtx, streamCancel := context.WithCancel(ctx)
	streamPayloadChan, streamErrChan := flux.ToChan(streamCtx, types.DefaultStreamBufferSize)

	firstPayload, err := recvStreamNext(streamCtx, streamPayloadChan, streamErrChan)
	if err != nil {
		streamCancel()
		return nil, nil, nil, err
	}
	firstResponse, err := rocket.DecodeResponsePayload(firstPayload)
	if err != nil {
		streamCancel()
		return nil, nil, nil, err
	}

	streamSeq := func(yield func(ReadableStruct, error) bool) {
		defer streamCancel()

		for {
			streamPayload, streamErr := recvStreamNext(streamCtx, streamPayloadChan, streamErrChan)
			if streamErr != nil {
				yield(nil, streamErr)
				return
			} else if streamPayload != nil {
				streamResponse, err := rocket.DecodeStreamPayload(streamPayload)
				if err != nil {
					yield(nil, err)
					return
				}
				data := streamResponse.Data()
				reader := bytes.NewBuffer(data)
				var decoder types.Decoder
				switch r.protoID {
				case rpcmetadata.ProtocolId_BINARY:
					decoder = format.NewBinaryDecoder(reader)
				case rpcmetadata.ProtocolId_COMPACT:
					decoder = format.NewCompactDecoder(reader)
				default:
					yield(nil, types.NewProtocolException(fmt.Errorf("Unknown protocol id: %d", r.protoID)))
					return
				}
				destStruct := newStreamElemFn()
				err = destStruct.Read(decoder)
				if err != nil {
					yield(nil, err)
					return
				} else if destEx := destStruct.Exception(); destEx != nil {
					yield(nil, destEx)
					return
				}

				if !yield(destStruct, nil) {
					return
				}
			} else {
				// Stream completion
				return
			}
		}
	}

	return firstResponse.Headers(), firstResponse.Data(), streamSeq, nil
}

func (r *rsocketClient) MetadataPush(_ context.Context, metadata *rpcmetadata.ClientPushMetadata) error {
	r.resetDeadline()
	payload, err := rocket.EncodePayloadMetadataAndData(metadata, nil, 0)
	if err != nil {
		return err
	}
	r.client.MetadataPush(payload)
	return nil
}

func (r *rsocketClient) RequestChannel(
	ctx context.Context,
	messageName string,
	headers map[string]string,
	dataBytes []byte,
) (map[string]string, []byte, func(sinkSeq iter.Seq2[WritableResult, error], finalResponse ReadableStruct) error, error) {
	r.resetDeadline()

	request, err := rocket.EncodeRequestPayload(
		ctx,
		messageName,
		r.protoID,
		rpcmetadata.RpcKind_SINK,
		headers,
		rpcmetadata.CompressionAlgorithm_NONE,
		dataBytes,
	)
	if err != nil {
		return nil, nil, nil, err
	}

	// Create a flux that yields the initial request payload and then waits for sink items
	sinkPayloadChan := make(chan payload.Payload, types.DefaultStreamBufferSize)
	sendingDoneChan := make(chan struct{}, 1)
	sendingFlux := flux.Create(func(fluxCtx context.Context, sink flux.Sink) {
		defer close(sendingDoneChan)

		// Send payloads from the sink channel until it's closed or context is done
		for {
			select {
			case p, ok := <-sinkPayloadChan:
				if !ok {
					// Channel closed, complete the flux
					sink.Complete()
					return
				}
				sink.Next(p)
			case <-fluxCtx.Done():
				sink.Error(fluxCtx.Err())
				return
			}
		}
	})

	receivingFlux := r.client.RequestChannel(request, sendingFlux)

	channelCtx, channelCancel := context.WithCancel(ctx)
	receivingPayloadChan, receivingErrChan := receivingFlux.ToChan(channelCtx, types.DefaultStreamBufferSize)

	// Receive the first response payload
	firstPayload, err := recvStreamNext(channelCtx, receivingPayloadChan, receivingErrChan)
	if err != nil {
		channelCancel()
		close(sinkPayloadChan)
		return nil, nil, nil, err
	}
	firstResponse, err := rocket.DecodeResponsePayload(firstPayload)
	if err != nil {
		channelCancel()
		close(sinkPayloadChan)
		return nil, nil, nil, err
	}

	// Create sink callback that will be called by the user to send sink items
	sinkCallback := func(sinkSeq iter.Seq2[WritableResult, error], finalResponse ReadableStruct) error {
		defer channelCancel()

		sendAllSinkItems := func() error {
			defer func() {
				close(sinkPayloadChan)
				<-sendingDoneChan
			}()

			for item, itemErr := range sinkSeq {
				var finalErr error
				payloadMetadata := rpcmetadata.NewPayloadMetadata()
				if itemErr != nil {
					exceptionMetadataBase := rocket.NewPayloadExceptionMetadataBase(
						"ApplicationException",
						itemErr.Error(),
						rocket.RocketExceptionAppUnknown,
						rpcmetadata.ErrorKind_UNSPECIFIED,
						rpcmetadata.ErrorBlame_UNSPECIFIED,
						rpcmetadata.ErrorSafety_UNSPECIFIED,
					)
					payloadMetadata.SetExceptionMetadata(exceptionMetadataBase)
					finalErr = itemErr
				} else if declaredException := item.Exception(); declaredException != nil {
					exceptionMetadataBase := rocket.NewPayloadExceptionMetadataBase(
						declaredException.TypeName(),
						declaredException.Error(),
						rocket.RocketExceptionDeclared,
						rpcmetadata.ErrorKind_UNSPECIFIED,
						rpcmetadata.ErrorBlame_UNSPECIFIED,
						rpcmetadata.ErrorSafety_UNSPECIFIED,
					)
					payloadMetadata.SetExceptionMetadata(exceptionMetadataBase)
					finalErr = declaredException
				} else {
					responseMetadata := rpcmetadata.NewPayloadResponseMetadata()
					payloadMetadata.SetResponseMetadata(responseMetadata)
				}

				metadata := rpcmetadata.NewStreamPayloadMetadata().
					SetCompression(Pointerize(rpcmetadata.CompressionAlgorithm_NONE)).
					SetPayloadMetadata(payloadMetadata)

				// Encode the sink item
				var itemBytes []byte
				if item != nil {
					itemBytes, err = encodeRequest(r.thriftProtoID, item)
					if err != nil {
						return err
					}
				}

				sinkPayload, err := rocket.EncodePayloadMetadataAndData(metadata, itemBytes, rpcmetadata.CompressionAlgorithm_NONE)
				if err != nil {
					return err
				}

				select {
				case sinkPayloadChan <- sinkPayload:
				case <-ctx.Done():
					return ctx.Err()
				}

				if finalErr != nil {
					return finalErr
				}
			}
			return nil
		}
		err = sendAllSinkItems()
		if err != nil {
			return err
		}

		// Wait for the final response
		finalPayload, err := recvStreamNext(channelCtx, receivingPayloadChan, receivingErrChan)
		if err != nil {
			return err
		}
		if finalPayload == nil {
			return fmt.Errorf("expected final response but channel completed")
		}

		finalRespPayload, err := rocket.DecodeStreamPayload(finalPayload)
		if err != nil {
			return err
		}

		// Decode the final response
		return decodeResponse(r.thriftProtoID, finalRespPayload.Data(), finalResponse)
	}

	return firstResponse.Headers(), firstResponse.Data(), sinkCallback, nil
}

func (r *rsocketClient) Close() error {
	defer r.clientScheduler.Close()
	if r.client != nil {
		return r.client.Close()
	}
	return r.conn.Close()
}

func recvStreamNext(ctx context.Context, streamPayloadChan <-chan payload.Payload, streamErrChan <-chan error) (payload.Payload, error) {
	// The logic below allows us to deal with channel-close race conditions
	// in a graceful manner. It's possible for any (or both) of the channels
	// to get closed before all the bufferred data is received from them.
	// The challenge is to figure out if the other channel still has data
	// to receive if one channel becomes closed. We achieve this by setting
	// a channel to nil (nil = not ready) and performing another iteration.
	// (https://stackoverflow.com/a/13666733)

	for range 2 /* max 2 iterations */ {
		select {
		case streamPayload, ok := <-streamPayloadChan:
			if ok {
				return streamPayload, nil
			}
			streamPayloadChan = nil
		case err, ok := <-streamErrChan:
			if ok {
				return nil, err
			}
			streamErrChan = nil
		case <-ctx.Done():
			return nil, ctx.Err()
		}
	}

	// (nil, nil) indicates completion - i.e. both channels confirmed closed.
	return nil, nil
}
