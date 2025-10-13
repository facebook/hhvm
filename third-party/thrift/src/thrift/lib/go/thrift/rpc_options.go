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
	"maps"
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
	mutex        sync.RWMutex
	writeHeaders map[string]string
	readHeaders  map[string]string
}

func (c *contextHeaders) GetReadHeaders() map[string]string {
	c.mutex.RLock()
	defer c.mutex.RUnlock()
	res := map[string]string{}
	maps.Copy(res, c.readHeaders)
	return res
}

func (c *contextHeaders) GetWriteHeaders() map[string]string {
	c.mutex.RLock()
	defer c.mutex.RUnlock()
	res := map[string]string{}
	maps.Copy(res, c.writeHeaders)
	return res
}

func (c *contextHeaders) SetWriteHeader(k, v string) {
	c.mutex.Lock()
	defer c.mutex.Unlock()
	if c.writeHeaders == nil {
		c.writeHeaders = map[string]string{}
	}
	c.writeHeaders[k] = v
}

func (c *contextHeaders) SetReadHeader(k, v string) {
	c.mutex.Lock()
	defer c.mutex.Unlock()
	if c.readHeaders == nil {
		c.readHeaders = map[string]string{}
	}
	c.readHeaders[k] = v
}

func (c *contextHeaders) SetWriteHeaders(headers map[string]string) {
	c.mutex.Lock()
	defer c.mutex.Unlock()
	c.writeHeaders = headers
}

func (c *contextHeaders) SetReadHeaders(headers map[string]string) {
	c.mutex.Lock()
	defer c.mutex.Unlock()
	c.readHeaders = headers
}

func (c *contextHeaders) GetWriteHeader(k string) (string, bool) {
	c.mutex.RLock()
	defer c.mutex.RUnlock()
	v, ok := c.writeHeaders[k]
	return v, ok
}

func (c *contextHeaders) GetReadHeader(k string) (string, bool) {
	c.mutex.RLock()
	defer c.mutex.RUnlock()
	v, ok := c.readHeaders[k]
	return v, ok
}

type rpcOptionsKeyType int

const (
	rpcOptionsKey rpcOptionsKeyType = 1
)

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
