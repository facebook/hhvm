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
	"net"
	"time"

	rsocket "github.com/rsocket/rsocket-go"
	"github.com/rsocket/rsocket-go/core/transport"
	"github.com/rsocket/rsocket-go/payload"
)

type rsocketClient struct {
	client rsocket.Client
	conn   net.Conn
}

type onServerMetadataPush func(*serverMetadataPayload)

func newRsocketClient(conn net.Conn, serverMetadataPush onServerMetadataPush) (*rsocketClient, error) {
	setupPayload, err := newRequestSetupPayloadVersion8()
	if err != nil {
		return nil, err
	}
	clientBuilder := rsocket.Connect()
	// See T182939211. This copies the keep alives from Java Rocket.
	// KeepaliveLifetime = time.Duration(missedAcks = 1) * (ackTimeout = 3600000)
	clientBuilder = clientBuilder.KeepAlive(time.Millisecond*30000, time.Millisecond*3600000, 1)
	clientBuilder = clientBuilder.SetupPayload(setupPayload)
	clientBuilder = clientBuilder.OnClose(func(error) {})
	clientStarter := clientBuilder.Acceptor(acceptor(serverMetadataPush))
	client, err := clientStarter.Transport(transporter(conn)).Start(context.Background())
	return &rsocketClient{client: client, conn: conn}, err
}

func acceptor(onMetadataPush onServerMetadataPush) func(_ context.Context, socket rsocket.RSocket) rsocket.RSocket {
	return func(_ context.Context, socket rsocket.RSocket) rsocket.RSocket {
		return rsocket.NewAbstractSocket(
			rsocket.MetadataPush(
				metadataPush(onMetadataPush),
			),
		)
	}
}

func metadataPush(onMetadataPush onServerMetadataPush) func(pay payload.Payload) {
	return func(pay payload.Payload) {
		metadata, err := decodeServerMetadataPushVersion8(pay)
		if err != nil {
			panic(err)
		}
		onMetadataPush(metadata)
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

func (r *rsocketClient) requestResponse(ctx context.Context, messageName string, protoID ProtocolID, typeID MessageType, headers map[string]string, persistentHeaders map[string]string, zstd bool, dataBytes []byte) (*responsePayload, error) {
	r.resetDeadline()
	request, err := encodeRequestPayload(messageName, protoID, typeID, headers, persistentHeaders, zstd, dataBytes)
	if err != nil {
		return nil, err
	}
	mono := r.client.RequestResponse(request)
	val, err := mono.Block(ctx)
	val = payload.Clone(val)
	if err != nil {
		return nil, err
	}
	return decodeResponsePayload(val)
}

func (r *rsocketClient) fireAndForget(messageName string, protoID ProtocolID, typeID MessageType, headers map[string]string, persistentHeaders map[string]string, zstd bool, dataBytes []byte) error {
	r.resetDeadline()
	request, err := encodeRequestPayload(messageName, protoID, typeID, headers, persistentHeaders, zstd, dataBytes)
	if err != nil {
		return err
	}
	r.client.FireAndForget(request)
	return nil
}

func (r *rsocketClient) Close() error {
	return r.client.Close()
}
