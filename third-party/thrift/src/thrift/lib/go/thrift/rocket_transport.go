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

	rsocket "github.com/rsocket/rsocket-go"
	"github.com/rsocket/rsocket-go/core/transport"
)

type rocketTransportFactory struct{}

// NewRocketTransportFactory makes it possible to create a RocketTransport using a thrift.Socket
func NewRocketTransportFactory() TransportFactory {
	return &rocketTransportFactory{}
}

func (p *rocketTransportFactory) GetTransport(socket Transport) Transport {
	tsocket, ok := socket.(*Socket)
	if !ok {
		panic(NewTransportException(
			NOT_IMPLEMENTED,
			fmt.Sprintf("Rocket Transport only supports Socket and does not support: %T", socket)))
	}
	return NewRocketTransport(tsocket)
}

type rocketTransport struct {
	socket *Socket

	// rsocket client state
	ctx    context.Context
	cancel func()
	client rsocket.Client

	// Used on read
	rBuf byteReader
	// remaining bytes in the current frame. If 0, read in a new frame.
	rSize uint64

	// Used on write
	wbuf *bytes.Buffer

	// Negotiated
	protoID ProtocolID
}

// NewRocketTransport creates a new transport with defaults.
func NewRocketTransport(socket *Socket) Transport {
	t := &rocketTransport{
		socket:  socket,
		protoID: ProtocolIDCompact,
	}
	t.resetBuffers()
	return t
}

func (t *rocketTransport) ProtocolID() ProtocolID {
	return t.protoID
}

func (t *rocketTransport) SetProtocolID(protoID ProtocolID) error {
	if !(protoID == ProtocolIDBinary || protoID == ProtocolIDCompact) {
		return NewTransportException(
			NOT_IMPLEMENTED, fmt.Sprintf("unimplemented proto ID: %s (%#x)", protoID.String(), int64(protoID)),
		)
	}
	t.protoID = protoID
	return nil
}

func (t *rocketTransport) resetBuffers() {
	t.rBuf = newLimitedByteReader(bytes.NewReader(nil), 0)
	t.rSize = 0
	t.wbuf = bytes.NewBuffer(nil)
}

// Open opens the internal transport (required for Transport)
func (t *rocketTransport) Open() error {
	if !t.socket.IsOpen() {
		if err := t.socket.Open(); err != nil {
			return err
		}
	}
	conn := t.socket.Conn()
	setupPayload, err := newRequestSetupPayloadVersion8()
	if err != nil {
		return err
	}
	transporter := func(ctx context.Context) (*transport.Transport, error) {
		return transport.NewTCPClientTransport(conn), nil
	}
	t.ctx, t.cancel = context.WithCancel(context.Background())
	t.client, err = rsocket.Connect().
		SetupPayload(setupPayload).
		Transport(transporter).
		Start(t.ctx)
	return err
}

// IsOpen returns whether the current transport is open (required for Transport)
func (t *rocketTransport) IsOpen() bool {
	return t.socket.IsOpen() && t.client != nil
}

// Close closes the internal transport (required for Transport)
func (t *rocketTransport) Close() error {
	if err := t.socket.Close(); err != nil {
		return err
	}
	defer t.cancel()
	if err := t.client.Close(); err != nil {
		return err
	}
	t.client = nil
	return nil
}

// Read reads from the current rBuffer. EOF if the frame is done. (required for Transport)
func (t *rocketTransport) Read(buf []byte) (int, error) {
	n, err := t.rBuf.Read(buf)
	// Shouldn't be possible, but just in case the frame size was flubbed
	if uint64(n) > t.rSize {
		n = int(t.rSize)
	}
	t.rSize -= uint64(n)
	return n, err
}

func (t *rocketTransport) setReadBuf(buf []byte) {
	t.rSize = uint64(len(buf))
	t.rBuf = newLimitedByteReader(bytes.NewBuffer(buf), int64(len(buf)))
}

// Write writes multiple bytes to the rBuffer, does not send to transport. (required for Transport)
func (t *rocketTransport) Write(buf []byte) (int, error) {
	n, err := t.wbuf.Write(buf)
	return n, NewTransportExceptionFromError(err)
}

// RemainingBytes returns how many bytes remain in the current recv rBuffer. (required for Transport)
// Used by binary and compact protocols in ReadString, ReadBytes, etc.,
// This is a defense tactic, so they can't get attacked with large messages and over allocate a buffer.
func (t *rocketTransport) RemainingBytes() uint64 {
	return t.rSize
}

// Flush (required for Transport)
func (t *rocketTransport) Flush() error {
	if err := t.socket.Flush(); err != nil {
		return err
	}
	t.resetBuffers()
	return nil
}
