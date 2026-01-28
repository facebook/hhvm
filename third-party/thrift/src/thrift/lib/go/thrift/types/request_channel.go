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

package types

import (
	"context"
	"io"
	"iter"
	"runtime"
	"sync/atomic"
)

// DefaultStreamBufferSize is the default size of the buffered channel used for streaming.
const DefaultStreamBufferSize = 100

// RequestChannel is an API that implements the most minimal surface for
// generated client code. An implementation:
//   - Must be thread-safe
//   - May implement request pipelining
//   - May implement reconnection logic
//   - May implement connection pooling
//   - Hides implementation details of the protocol and transport.
type RequestChannel interface {
	io.Closer

	SendRequestResponse(
		ctx context.Context,
		method string,
		request WritableStruct,
		response ReadableStruct,
	) error
	SendRequestNoResponse(
		ctx context.Context,
		method string,
		request WritableStruct,
	) error
	SendRequestStream(
		ctx context.Context,
		method string,
		request WritableStruct,
		response ReadableStruct,
		newStreamElemFn func() ReadableResult,
		onStreamNextFn func(ReadableStruct),
		onStreamErrorFn func(error),
		onStreamCompleteFn func(),
	) (iter.Seq2[ReadableStruct, error], error)
}

// RequestChannelExtended will eventually become part of RequestChannel, once legacy clients are gone (e.g. Header)
type RequestChannelExtended interface {
	RequestChannel
	TerminateInteraction(interactionID int64) error
}

// Monotonoically increasing interaction ID generator.
var nextInteractionID atomic.Int64

// interactionChannel is a thin wrapper around RequestChannel that enabled interactions.
type interactionChannel struct {
	channel            RequestChannel
	interactionName    string
	interactionID      int64
	interactionCreated bool
	cleanup            runtime.Cleanup
}

// Compile time interface enforcer
var _ RequestChannel = (*interactionChannel)(nil)

// NewInteractionChannel creates a new interaction channel.
func NewInteractionChannel(channel RequestChannel, interactionName string) RequestChannel {
	interactionID := nextInteractionID.Add(1)
	result := &interactionChannel{
		channel:            channel,
		interactionName:    interactionName,
		interactionID:      interactionID,
		interactionCreated: false,
	}
	result.cleanup = runtime.AddCleanup(result,
		func(channel RequestChannel) {
			sendTerminationSignal(channel, interactionID)
		}, channel)
	return result
}

func (c *interactionChannel) SendRequestResponse(
	ctx context.Context,
	method string,
	request WritableStruct,
	response ReadableStruct,
) error {
	ctx = c.withInteractionContext(ctx)
	return c.channel.SendRequestResponse(ctx, method, request, response)
}

func (c *interactionChannel) SendRequestNoResponse(
	ctx context.Context,
	method string,
	request WritableStruct,
) error {
	ctx = c.withInteractionContext(ctx)
	return c.channel.SendRequestNoResponse(ctx, method, request)
}

func (c *interactionChannel) SendRequestStream(
	ctx context.Context,
	method string,
	request WritableStruct,
	response ReadableStruct,
	newStreamElemFn func() ReadableResult,
	onStreamNextFn func(ReadableStruct),
	onStreamErrorFn func(error),
	onStreamCompleteFn func(),
) (iter.Seq2[ReadableStruct, error], error) {
	ctx = c.withInteractionContext(ctx)
	return c.channel.SendRequestStream(ctx, method, request, response, newStreamElemFn, onStreamNextFn, onStreamErrorFn, onStreamCompleteFn)
}

func (c *interactionChannel) Close() error {
	sendTerminationSignal(c.channel, c.interactionID)
	// no need for the cleanup anymore
	c.cleanup.Stop()
	// Do not close the underlying channel, we don't own it.
	// It may still be used by someone outside of the interaction.
	return nil
}

func sendTerminationSignal(channel RequestChannel, iteractionID int64) {
	if extChannel, ok := channel.(RequestChannelExtended); ok {
		// Best effort. Nothing we can do about the error.
		_ = extChannel.TerminateInteraction(iteractionID)
	}
}

func (c *interactionChannel) withInteractionContext(ctx context.Context) context.Context {
	if !c.interactionCreated {
		ctx = context.WithValue(ctx, interactionCreateKey, c.interactionName)
		c.interactionCreated = true
	}
	ctx = context.WithValue(ctx, interactionIDKey, c.interactionID)
	return ctx
}

func GetInteractionIDFromContext(ctx context.Context) (int64, bool) {
	interactionID, ok := ctx.Value(interactionIDKey).(int64)
	return interactionID, ok
}

func GetInteractionCreateFromContext(ctx context.Context) (string, bool) {
	interactionName, ok := ctx.Value(interactionCreateKey).(string)
	return interactionName, ok
}
