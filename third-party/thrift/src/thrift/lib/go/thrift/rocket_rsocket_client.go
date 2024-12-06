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
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	rsocket "github.com/rsocket/rsocket-go"
	"github.com/rsocket/rsocket-go/core/transport"
	"github.com/rsocket/rsocket-go/payload"
)

// RSocketClient is a client that uses a rsocket library.
type RSocketClient interface {
	SendSetup(serverMetadataPush OnServerMetadataPush) error
	FireAndForget(messageName string, protoID types.ProtocolID, typeID types.MessageType, headers map[string]string, zstd bool, dataBytes []byte) error
	RequestResponse(ctx context.Context, messageName string, protoID types.ProtocolID, typeID types.MessageType, headers map[string]string, zstd bool, dataBytes []byte) (map[string]string, []byte, error)
	Close() error
}

type rsocketClient struct {
	client rsocket.Client
	conn   net.Conn
}

// OnServerMetadataPush is called when the server sends a metadata push.
type OnServerMetadataPush func(zstd bool, drain bool)

func newRSocketClient(conn net.Conn) RSocketClient {
	return &rsocketClient{conn: conn}
}

func (r *rsocketClient) SendSetup(serverMetadataPush OnServerMetadataPush) error {
	if r.client != nil {
		// already setup
		return nil
	}
	setupPayload, err := newRequestSetupPayloadVersion8()
	if err != nil {
		return err
	}
	// See T182939211. This copies the keep alives from Java Rocket.
	// KeepaliveLifetime = time.Duration(missedAcks = 1) * (ackTimeout = 3600000)
	clientBuilder := rsocket.Connect().
		KeepAlive(time.Millisecond*30000, time.Millisecond*3600000, 1).
		SetupPayload(setupPayload).
		OnClose(func(error) {})
	clientStarter := clientBuilder.Acceptor(acceptor(serverMetadataPush))
	client, err := clientStarter.Transport(transporter(r.conn)).Start(context.Background())
	r.client = client
	if client == nil && err == nil {
		return fmt.Errorf("rsocket returnen nil client")
	}
	return err
}

func acceptor(onMetadataPush OnServerMetadataPush) func(_ context.Context, socket rsocket.RSocket) rsocket.RSocket {
	return func(_ context.Context, socket rsocket.RSocket) rsocket.RSocket {
		return rsocket.NewAbstractSocket(
			rsocket.MetadataPush(
				metadataPush(onMetadataPush),
			),
		)
	}
}

func metadataPush(onMetadataPush OnServerMetadataPush) func(pay payload.Payload) {
	return func(pay payload.Payload) {
		metadata, err := decodeServerMetadataPushVersion8(pay)
		if err != nil {
			panic(err)
		}
		onMetadataPush(metadata.zstd, metadata.drain)
	}
}

func transporter(conn net.Conn) func(_ context.Context) (*transport.Transport, error) {
	return func(_ context.Context) (*transport.Transport, error) {
		tconn := transport.NewTCPClientTransport(conn)
		tconn.SetLifetime(time.Millisecond * 3600000)
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

func (r *rsocketClient) RequestResponse(ctx context.Context, messageName string, protoID types.ProtocolID, typeID types.MessageType, headers map[string]string, zstd bool, dataBytes []byte) (map[string]string, []byte, error) {
	r.resetDeadline()
	request, err := encodeRequestPayload(messageName, protoID, typeID, headers, zstd, dataBytes)
	if err != nil {
		return nil, nil, err
	}
	val, err := rsocketBlock(ctx, r.client, request)
	if err != nil {
		return nil, nil, err
	}
	response, err := decodeResponsePayload(val)
	if response != nil {
		return response.Headers(), response.Data(), err
	}
	return nil, nil, err
}

func (r *rsocketClient) FireAndForget(messageName string, protoID types.ProtocolID, typeID types.MessageType, headers map[string]string, zstd bool, dataBytes []byte) error {
	r.resetDeadline()
	request, err := encodeRequestPayload(messageName, protoID, typeID, headers, zstd, dataBytes)
	if err != nil {
		return err
	}
	r.client.FireAndForget(request)
	return nil
}

func (r *rsocketClient) Close() error {
	if r.client != nil {
		return r.client.Close()
	}
	return r.conn.Close()
}
