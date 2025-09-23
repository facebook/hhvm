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
	"sync"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

// serialChannel is a simple threadsafe channel which allows for a single
// request-response to occur at once. Head of line blocking can occur with this channel.
type serialChannel struct {
	protocol Protocol
	seqID    int32
	lock     sync.Mutex
}

// newSerialChannel creates a new serial channel.
// The protocol should already be open.
func newSerialChannel(protocol Protocol) RequestChannel {
	return &serialChannel{
		protocol: protocol,
	}
}

func (c *serialChannel) sendMsg(ctx context.Context, method string, request WritableStruct, msgType MessageType) (int32, error) {
	c.seqID++
	seqID := c.seqID

	if err := SetRequestHeaders(ctx, c.protocol); err != nil {
		return seqID, err
	}

	if err := c.protocol.WriteMessageBegin(method, msgType, seqID); err != nil {
		return seqID, err
	}

	if err := request.Write(c.protocol); err != nil {
		return seqID, err
	}

	if err := c.protocol.WriteMessageEnd(); err != nil {
		return seqID, err
	}

	return seqID, c.protocol.Flush()
}

func (c *serialChannel) recvMsg(ctx context.Context, method string, seqID int32, response ReadableStruct) error {
	// TODO: Implement per-call cancellation for a serialChannel
	recvMethod, mTypeID, msgSeqID, err := c.protocol.ReadMessageBegin()
	if err != nil {
		return err
	}

	if method != recvMethod {
		return NewApplicationException(WRONG_METHOD_NAME, fmt.Sprintf("%s failed: wrong method name", method))
	}

	if msgSeqID != seqID {
		return NewApplicationException(types.BAD_SEQUENCE_ID, fmt.Sprintf("%s failed: out of sequence response", method))
	}

	switch mTypeID {
	case REPLY:
		if err := response.Read(c.protocol); err != nil {
			return err
		}
		if err := c.protocol.ReadMessageEnd(); err != nil {
			return err
		}
		responseHeaders := c.protocol.getResponseHeaders()
		setResponseHeaders(ctx, responseHeaders)
		return nil
	case EXCEPTION:
		appException := NewApplicationException(UNKNOWN_APPLICATION_EXCEPTION, "Unknown exception")
		if err := appException.Read(c.protocol); err != nil {
			return err
		}
		if err := c.protocol.ReadMessageEnd(); err != nil {
			return err
		}
		responseHeaders := c.protocol.getResponseHeaders()
		setResponseHeaders(ctx, responseHeaders)
		return appException
	default:
		return NewApplicationException(INVALID_MESSAGE_TYPE_EXCEPTION, fmt.Sprintf("%s failed: invalid message type", method))
	}
}

// Close closes the client connection
func (c *serialChannel) Close() error {
	c.lock.Lock()
	defer c.lock.Unlock()
	return c.protocol.Close()
}

// SendRequestResponse will call the given method with the given thrift struct, and read the response
// into the given response struct. It only allows one outstanding request at once, but is thread-safe.
func (c *serialChannel) SendRequestResponse(ctx context.Context, method string, request WritableStruct, response ReadableStruct) error {
	c.lock.Lock()
	defer c.lock.Unlock()

	seqID, err := c.sendMsg(ctx, method, request, CALL)
	if err != nil {
		return err
	}

	err = c.recvMsg(ctx, method, seqID, response)
	if err != nil {
		return err
	}

	return nil
}

// SendRequestNoResponse will call the given method with the given thrift struct. It returns immediately when the request is sent.
// It only allows one outstanding request at once, but is thread-safe.
func (c *serialChannel) SendRequestNoResponse(ctx context.Context, method string, request WritableStruct) error {
	c.lock.Lock()
	defer c.lock.Unlock()

	_, err := c.sendMsg(ctx, method, request, ONEWAY)
	if err != nil {
		return err
	}

	return nil
}

// SendRequestStream performs a request-stream call.
func (c *serialChannel) SendRequestStream(
	ctx context.Context,
	method string,
	request WritableStruct,
	response ReadableStruct,
	onStreamNextFn func(Decoder) error,
	onStreamErrorFn func(error),
	onStreamCompleteFn func(),
) error {
	return fmt.Errorf("not implemented")
}
