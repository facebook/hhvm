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
)

// ClientConn holds all the connection information for a thrift client
type ClientConn struct {
	proto Protocol
	seqID int32
}

// NewClientConn creates a new ClientConn object using a protocol
func NewClientConn(proto Protocol) ClientConn {
	return ClientConn{
		proto: proto,
	}
}

// IRequest represents a request to be sent to a thrift endpoint
type IRequest interface {
	Write(p Format) error
}

// IResponse represents a response received from a thrift call
type IResponse interface {
	Read(p Format) error
}

// Close closes the client connection
func (cc *ClientConn) Close() error {
	return cc.proto.Close()
}

// SendMsg sends a request to a given thrift endpoint
func (cc *ClientConn) SendMsg(ctx context.Context, method string, req IRequest, msgType MessageType) error {
	cc.seqID++

	if err := setRequestHeaders(ctx, cc.proto); err != nil {
		return err
	}

	if err := cc.proto.WriteMessageBegin(method, msgType, cc.seqID); err != nil {
		return err
	}

	if err := req.Write(cc.proto); err != nil {
		return err
	}

	if err := cc.proto.WriteMessageEnd(); err != nil {
		return err
	}

	return cc.proto.Flush()
}

// RecvMsg receives the response from a call to a thrift endpoint
func (cc *ClientConn) RecvMsg(ctx context.Context, method string, res IResponse) error {
	recvMethod, mTypeID, seqID, err := cc.proto.ReadMessageBegin()

	if err != nil {
		return err
	}

	if method != recvMethod {
		return NewApplicationException(WRONG_METHOD_NAME, fmt.Sprintf("%s failed: wrong method name", method))
	}

	if cc.seqID != seqID {
		return NewApplicationException(BAD_SEQUENCE_ID, fmt.Sprintf("%s failed: out of sequence response", method))
	}

	switch mTypeID {
	case REPLY:
		if err := res.Read(cc.proto); err != nil {
			return err
		}

		return cc.proto.ReadMessageEnd()
	case EXCEPTION:
		err := NewApplicationException(UNKNOWN_APPLICATION_EXCEPTION, "Unknown exception")

		recvdErr, readErr := err.Read(cc.proto)

		if readErr != nil {
			return readErr
		}

		if msgEndErr := cc.proto.ReadMessageEnd(); msgEndErr != nil {
			return msgEndErr
		}
		return recvdErr
	default:
		return NewApplicationException(INVALID_MESSAGE_TYPE_EXCEPTION, fmt.Sprintf("%s failed: invalid message type", method))
	}
}
