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
	"math"
	"net"
	"time"

	"golang.org/x/sync/singleflight"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/rocket"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
	"github.com/jjeffcaii/reactor-go/scheduler"
	rsocket "github.com/rsocket/rsocket-go"
	"github.com/rsocket/rsocket-go/core/transport"
	"github.com/rsocket/rsocket-go/payload"
)

// RSocketClient is a client that uses a rsocket library.
type RSocketClient interface {
	SendSetup(ctx context.Context) error
	FireAndForget(
		messageName string,
		protoID types.ProtocolID,
		headers map[string]string,
		dataBytes []byte,
	) error
	RequestResponse(
		ctx context.Context,
		messageName string,
		protoID types.ProtocolID,
		headers map[string]string,
		dataBytes []byte,
	) (map[string]string, []byte, error)
	RequestStream(
		ctx context.Context,
		messageName string,
		protoID types.ProtocolID,
		headers map[string]string,
		dataBytes []byte,
		onStreamNextFn func([]byte) error,
		onStreamErrorFn func(error),
		onStreamComplete func(),
	) (map[string]string, []byte, error)
	Close() error
}

type rsocketClient struct {
	client rsocket.Client
	conn   net.Conn

	clientScheduler scheduler.Scheduler

	initGroup singleflight.Group

	useZstd bool
}

func newRSocketClient(conn net.Conn) RSocketClient {
	return &rsocketClient{
		conn:            conn,
		clientScheduler: scheduler.NewElastic(math.MaxInt32),
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
	protoID types.ProtocolID,
	headers map[string]string,
	dataBytes []byte,
) (map[string]string, []byte, error) {
	r.resetDeadline()
	request, err := rocket.EncodeRequestPayload(
		messageName,
		protoID,
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

func (r *rsocketClient) FireAndForget(messageName string, protoID types.ProtocolID, headers map[string]string, dataBytes []byte) error {
	r.resetDeadline()
	request, err := rocket.EncodeRequestPayload(
		messageName,
		protoID,
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
	protoID types.ProtocolID,
	headers map[string]string,
	dataBytes []byte,
	onStreamNextFn func([]byte) error,
	onStreamErrorFn func(error),
	onStreamComplete func(),
) (map[string]string, []byte, error) {
	r.resetDeadline()

	request, err := rocket.EncodeRequestPayload(
		messageName,
		protoID,
		rpcmetadata.RpcKind_SINGLE_REQUEST_STREAMING_RESPONSE,
		headers,
		rpcmetadata.CompressionAlgorithm_NONE,
		dataBytes,
	)
	if err != nil {
		return nil, nil, err
	}

	flux := r.client.RequestStream(request)

	streamCtx, streamCancel := context.WithCancel(ctx)
	streamPayloadChan, streamErrChan := flux.ToChan(streamCtx, types.DefaultStreamBufferSize)

	firstPayload, err := recvStreamNext(streamCtx, streamPayloadChan, streamErrChan)
	if err != nil {
		streamCancel()
		return nil, nil, err
	}
	firstResponse, err := rocket.DecodeResponsePayload(firstPayload)
	if err != nil {
		streamCancel()
		return nil, nil, err
	}

	go func() {
		defer streamCancel()

		for {
			streamPayload, streamErr := recvStreamNext(streamCtx, streamPayloadChan, streamErrChan)
			if streamErr != nil {
				onStreamErrorFn(streamErr)
				return
			} else if streamPayload != nil {
				streamResponse, err := rocket.DecodeStreamPayload(streamPayload)
				if err != nil {
					onStreamErrorFn(err)
					return
				}
				err = onStreamNextFn(streamResponse.Data())
				if err != nil {
					onStreamErrorFn(err)
					return
				}
			} else {
				// Stream completion
				onStreamComplete()
				return
			}
		}
	}()

	return firstResponse.Headers(), firstResponse.Data(), nil
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
