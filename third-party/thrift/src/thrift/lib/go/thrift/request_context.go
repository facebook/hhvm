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
	"sync"
	"time"
)

// RPCOptions is a mirror of C++ apache::thrift::RPCOptions
// Not all options are guaranteed to be implemented by a client
type RPCOptions struct {
	Timeout            time.Duration
	ChunkTimeout       time.Duration
	QueueTimeout       time.Duration
	OverallTimeout     time.Duration
	ProcessingTimeout  time.Duration
	Priority           Priority
	ClientOnlyTimeouts bool
	InteractionID      int64

	// For sending and receiving headers.
	contextHeaders
}

// Identity represents a secure peer identity
type Identity struct {
	Type string
	Data string
}

// RequestContext is a mirror of C++ apache::thrift::RPCOptions
// Not all options are guaranteed to be implemented by a client
type RequestContext struct {
	RequestTimeout time.Duration
	Method         string
	SequenceID     int32
	InteractionID  int64
	Priority       Priority

	LocalAddress     net.Addr
	PeerAddress      net.Addr
	SecurityProtocol string
	PeerIdentities   []Identity
	PeerCommonName   string

	// For sending and receiving headers.
	contextHeaders
}

// WithRPCOptions sets the RPCOptions in a client request go context
func WithRPCOptions(ctx context.Context, opts *RPCOptions) context.Context {
	return context.WithValue(ctx, rpcOptionsKey, opts)
}

// GetRPCOptions returns the RPCOptions in a go context, or nil if there is nothing
func GetRPCOptions(ctx context.Context) *RPCOptions {
	v := ctx.Value(rpcOptionsKey)
	if v == nil {
		return nil
	}
	return v.(*RPCOptions)
}

// GetRequestContext returns the RequestContext in a go context, or nil if there is nothing
func GetRequestContext(ctx context.Context) *RequestContext {
	v := ctx.Value(reqContextKey)
	if v == nil {
		return nil
	}
	return v.(*RequestContext)
}

// WithRequestContext sets the RequestContext in a given go context
func WithRequestContext(ctx context.Context, reqCtx *RequestContext) context.Context {
	return context.WithValue(ctx, reqContextKey, reqCtx)
}

type requestContextKey int

const (
	rpcOptionsKey requestContextKey = 1
	reqContextKey requestContextKey = 2
)

// Priority maps to C++ apache::thrift::concurrency::PRIORITY
type Priority uint8

// Priority maps to C++ apache::thrift::concurrency::PRIORITY
const (
	PriorityNone          Priority = 0
	PriorityHighImportant Priority = 1
	PriorityHigh          Priority = 2
	PriorityImportant     Priority = 3
	PriorityNormal        Priority = 4
	PriorityBestEffort    Priority = 5
	PriorityEnd           Priority = 6
)

type contextHeaders struct {
	writeHeaders    map[string]string
	readHeaders     map[string]string
	writeHeaderLock sync.Mutex
	readHeaderLock  sync.Mutex
}

func (c *contextHeaders) ReadHeaders() map[string]string {
	c.readHeaderLock.Lock()
	defer c.readHeaderLock.Unlock()
	res := map[string]string{}
	for k, v := range c.readHeaders {
		res[k] = v
	}
	return res
}

func (c *contextHeaders) WriteHeaders() map[string]string {
	c.writeHeaderLock.Lock()
	defer c.writeHeaderLock.Unlock()
	res := map[string]string{}
	for k, v := range c.writeHeaders {
		res[k] = v
	}
	return res
}

func (c *contextHeaders) SetWriteHeader(k, v string) {
	c.writeHeaderLock.Lock()
	defer c.writeHeaderLock.Unlock()
	if c.writeHeaders == nil {
		c.writeHeaders = map[string]string{}
	}
	c.writeHeaders[k] = v
}

func (c *contextHeaders) SetReadHeader(k, v string) {
	c.readHeaderLock.Lock()
	defer c.readHeaderLock.Unlock()
	if c.readHeaders == nil {
		c.readHeaders = map[string]string{}
	}
	c.readHeaders[k] = v
}

func (c *contextHeaders) SetWriteHeaders(headers map[string]string) {
	c.writeHeaderLock.Lock()
	defer c.writeHeaderLock.Unlock()
	c.writeHeaders = headers
}

func (c *contextHeaders) SetReadHeaders(headers map[string]string) {
	c.readHeaderLock.Lock()
	defer c.readHeaderLock.Unlock()
	c.readHeaders = headers
}

func (c *contextHeaders) GetWriteHeader(k string) (string, bool) {
	c.writeHeaderLock.Lock()
	defer c.writeHeaderLock.Unlock()
	v, ok := c.writeHeaders[k]
	return v, ok
}

func (c *contextHeaders) GetReadHeader(k string) (string, bool) {
	c.readHeaderLock.Lock()
	defer c.readHeaderLock.Unlock()
	v, ok := c.readHeaders[k]
	return v, ok
}
