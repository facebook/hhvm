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
)

// SerialChannel is a simple threadsafe channel which allows for a single
// request-response to occur at once. Head of line blocking can occur with this channel.
type SerialChannel struct {
	protocol Protocol
	seqID    int32
	lock     sync.Mutex
}

// NewSerialChannel creates a new serial channel.
// The protocol should already be open.
func NewSerialChannel(protocol Protocol) *SerialChannel {
	return &SerialChannel{
		protocol: protocol,
	}
}

func (c *SerialChannel) sendMsg(ctx context.Context, method string, request IRequest, msgType MessageType) (int32, error) {
	c.seqID++
	seqID := c.seqID

	if err := setHeaders(ctx, c.protocol); err != nil {
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

func (c *SerialChannel) recvMsg(method string, seqID int32, response IResponse) error {
	// TODO: Implement per-call cancellation for a SerialChannel
	recvMethod, mTypeID, msgSeqID, err := c.protocol.ReadMessageBegin()

	if err != nil {
		return err
	}

	if method != recvMethod {
		return NewApplicationException(WRONG_METHOD_NAME, fmt.Sprintf("%s failed: wrong method name", method))
	}

	if msgSeqID != seqID {
		return NewApplicationException(BAD_SEQUENCE_ID, fmt.Sprintf("%s failed: out of sequence response", method))
	}

	switch mTypeID {
	case REPLY:
		if err := response.Read(c.protocol); err != nil {
			return err
		}

		return c.protocol.ReadMessageEnd()
	case EXCEPTION:
		err := NewApplicationException(UNKNOWN_APPLICATION_EXCEPTION, "Unknown exception")

		recvdErr, readErr := err.Read(c.protocol)

		if readErr != nil {
			return readErr
		}

		if msgEndErr := c.protocol.ReadMessageEnd(); msgEndErr != nil {
			return msgEndErr
		}
		return recvdErr
	default:
		return NewApplicationException(INVALID_MESSAGE_TYPE_EXCEPTION, fmt.Sprintf("%s failed: invalid message type", method))
	}
}

// Close closes the client connection
func (c *SerialChannel) Close() error {
	return c.protocol.Close()
}

// IsOpen return true if the client connection is open; otherwise, it returns false.
func (c *SerialChannel) IsOpen() bool {
	return c.protocol.IsOpen()
}

// Open opens the client connection
func (c *SerialChannel) Open() error {
	return c.protocol.Open()
}

// Call will call the given method with the given thrift struct, and read the response
// into the given response struct. It only allows one outstanding request at once, but is thread-safe.
func (c *SerialChannel) Call(ctx context.Context, method string, request IRequest, response IResponse) error {
	c.lock.Lock()
	defer c.lock.Unlock()

	seqID, err := c.sendMsg(ctx, method, request, CALL)
	if err != nil {
		return err
	}

	err = c.recvMsg(method, seqID, response)
	if err != nil {
		return err
	}

	return nil
}

// Oneway will call the given method with the given thrift struct. It returns immediately when the request is sent.
// It only allows one outstanding request at once, but is thread-safe.
func (c *SerialChannel) Oneway(ctx context.Context, method string, request IRequest) error {
	c.lock.Lock()
	defer c.lock.Unlock()

	_, err := c.sendMsg(ctx, method, request, ONEWAY)
	if err != nil {
		return err
	}

	return nil
}
