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
	transport Transport
	iproto    Protocol
	oproto    Protocol
	seqID     int32
}

// Transport returns the underlying Transport object inside the ClientConn
// object
func (cc *ClientConn) Transport() Transport {
	return cc.transport
}

// NewClientConnWithProtocols creates a new ClientConn object using the input and output protocols provided
func NewClientConnWithProtocols(t Transport, iproto, oproto Protocol) ClientConn {
	return ClientConn{
		transport: t,
		iproto:    iproto,
		oproto:    oproto,
	}
}

// IRequest represents a request to be sent to a thrift endpoint
type IRequest interface {
	Write(p Protocol) error
}

// IResponse represents a response received from a thrift call
type IResponse interface {
	Read(p Protocol) error
}

// Close closes the client connection
func (cc *ClientConn) Close() error {
	return cc.oproto.Close()
}

// SendMsg sends a request to a given thrift endpoint
func (cc *ClientConn) SendMsg(ctx context.Context, method string, req IRequest, msgType MessageType) error {
	cc.seqID++

	if err := setHeaders(ctx, cc.oproto); err != nil {
		return err
	}

	if err := cc.oproto.WriteMessageBegin(method, msgType, cc.seqID); err != nil {
		return err
	}

	if err := req.Write(cc.oproto); err != nil {
		return err
	}

	if err := cc.oproto.WriteMessageEnd(); err != nil {
		return err
	}

	return cc.oproto.Flush()
}

// RecvMsg receives the response from a call to a thrift endpoint
func (cc *ClientConn) RecvMsg(ctx context.Context, method string, res IResponse) error {
	recvMethod, mTypeID, seqID, err := cc.iproto.ReadMessageBegin()

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
		if err := res.Read(cc.iproto); err != nil {
			return err
		}

		return cc.iproto.ReadMessageEnd()
	case EXCEPTION:
		err := NewApplicationException(UNKNOWN_APPLICATION_EXCEPTION, "Unknown exception")

		recvdErr, readErr := err.Read(cc.iproto)

		if readErr != nil {
			return readErr
		}

		if msgEndErr := cc.iproto.ReadMessageEnd(); msgEndErr != nil {
			return msgEndErr
		}
		return recvdErr
	default:
		return NewApplicationException(INVALID_MESSAGE_TYPE_EXCEPTION, fmt.Sprintf("%s failed: invalid message type", method))
	}
}
