/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

package main

import (
	"context"
	"fmt"
	"time"

	"apache/thrift/test/load"
	"thrift/lib/go/thrift"

	"github.com/golang/glog"
	"github.com/pkg/errors"
)

// Handler encapsulates parameters for your service processor
type Handler struct{}

// Serve initializes your thrift server
func Serve(addr string) error {
	handler := &Handler{}
	proc := load.NewLoadTestProcessor(handler)
	srv, err := newServer(proc, addr)

	if err != nil {
		return errors.Wrap(err, "failed to create thrift server")
	}

	glog.Infof("starting thrift server on '%s'", addr)
	return srv.Serve()
}

func newServer(processor thrift.ProcessorContext, addr string) (thrift.Server, error) {
	socket, err := thrift.NewServerSocket(addr)
	if err != nil {
		return nil, err
	}
	if err = socket.Listen(); err != nil {
		return nil, errors.Wrap(err, fmt.Sprintf("failed listen on %s", addr))
	}
	tFactory := thrift.NewHeaderTransportFactory(thrift.NewTransportFactory())
	pFactory := thrift.NewHeaderProtocolFactory()

	return thrift.NewSimpleServerContext(
		processor, socket, thrift.TransportFactories(tFactory), thrift.ProtocolFactories(pFactory)), nil
}

// Echo - does echo
func (h *Handler) Echo(ctx context.Context, in []byte) ([]byte, error) {
	return in, nil
}

// Add - adds
func (h *Handler) Add(ctx context.Context, a int64, b int64) (r int64, err error) {
	return a + b, nil
}

// Noop - a noop
func (h *Handler) Noop(ctx context.Context) (err error) {
	return nil
}

// OnewayNoop - a one way noop
func (h *Handler) OnewayNoop(ctx context.Context) (err error) {
	return nil
}

// AsyncNoop - a async noop
func (h *Handler) AsyncNoop(ctx context.Context) (err error) {
	return nil
}

// Sleep - a sleep
func (h *Handler) Sleep(ctx context.Context, microseconds int64) (err error) {
	time.Sleep(time.Duration(microseconds) * time.Microsecond)
	return nil
}

// OnewaySleep - a oneway sleep
func (h *Handler) OnewaySleep(ctx context.Context, microseconds int64) (err error) {
	time.Sleep(time.Duration(microseconds) * time.Microsecond)
	return nil
}

// Burn - a time burn
func (h *Handler) Burn(ctx context.Context, microseconds int64) (err error) {
	burnImpl(microseconds)
	return nil
}

// OnewayBurn - a one way time burn
func (h *Handler) OnewayBurn(ctx context.Context, microseconds int64) (err error) {
	burnImpl(microseconds)
	return nil
}

// BadSleep - a burn instead of a sleep
func (h *Handler) BadSleep(ctx context.Context, microseconds int64) (err error) {
	burnImpl(microseconds)
	return nil
}

// BadBurn - a time burn
func (h *Handler) BadBurn(ctx context.Context, microseconds int64) (err error) {
	burnImpl(microseconds)
	return nil
}

// ThrowError - throws a LoadError
func (h *Handler) ThrowError(ctx context.Context, code int32) (err error) {
	return load.NewLoadError()
}

// ThrowUnexpected - throws a ApplicationException
func (h *Handler) ThrowUnexpected(ctx context.Context, code int32) (err error) {
	return thrift.NewApplicationException(thrift.UNKNOWN_APPLICATION_EXCEPTION, "Unknown Exception")
}

// OnewayThrow - throws a ApplicationException
func (h *Handler) OnewayThrow(ctx context.Context, code int32) (err error) {
	return thrift.NewApplicationException(thrift.UNKNOWN_APPLICATION_EXCEPTION, "Unknown Exception")
}

// Send - does nothing
func (h *Handler) Send(ctx context.Context, data []byte) (err error) {
	return nil
}

// OnewaySend - does nothing
func (h *Handler) OnewaySend(ctx context.Context, data []byte) (err error) {
	return nil
}

// Recv - returns an empty byte array initialized to the size of the array
func (h *Handler) Recv(ctx context.Context, bytes int64) (r []byte, err error) {
	res := make([]byte, bytes)
	for n := range res {
		res[n] = byte('a')
	}
	return res, nil
}

// Sendrecv - returns a Recv invocation
func (h *Handler) Sendrecv(ctx context.Context, data []byte, recvBytes int64) (r []byte, err error) {
	if r, err = h.Recv(ctx, recvBytes); err != nil {
		return nil, err
	}
	return r, nil
}

// LargeContainer - does nothing
func (h *Handler) LargeContainer(ctx context.Context, items []*load.BigStruct) (err error) {
	return nil
}

// IterAllFields - iterates over all items and their fields
func (h *Handler) IterAllFields(ctx context.Context, items []*load.BigStruct) (r []*load.BigStruct, err error) {
	for _, item := range items {
		_ = item.GetStringField()
		for range item.GetStringList() {
		}
	}

	return items, nil
}

func burnImpl(microseconds int64) {
	end := time.Now().UnixNano() + microseconds*1000
	for time.Now().UnixNano() < end {
	}
}
