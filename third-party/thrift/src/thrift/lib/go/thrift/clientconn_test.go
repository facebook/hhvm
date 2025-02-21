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
	"errors"
	"fmt"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

var errFakeResponseRead = errors.New("error reading from FakeResponse")
var errFakeRequestWrite = errors.New("error writing FakeRequest")
var errFakeProtoReadMessageBegin = errors.New("error reading message begin from FakeProto")
var errFakeProtoWriteMessageBegin = errors.New("error writing message begin from FakeProto")
var errFakeProtoWriteMessageEnd = errors.New("error writing message end from FakeProto")
var errFakeProtoFlush = errors.New("error flushing FakeProto")

type fakeResponse struct {
	types.ReadableStruct
	shouldReturnError bool
}

type fakeRequest struct {
	types.WritableStruct
	shouldReturnError bool
}

type fakeProto struct {
	Protocol
	method            string
	typeID            types.MessageType
	seqID             int32
	shouldReturnError bool
	errOnMessageBegin bool
	errOnMessageEnd   bool
	errOnFlush        bool
}

func (f *fakeRequest) Write(proto types.Encoder) error {
	if f.shouldReturnError {
		return errFakeRequestWrite
	}
	return nil
}

func (f *fakeProto) WriteMessageBegin(method string, typeID types.MessageType, seqID int32) error {
	if f.errOnMessageBegin {
		return errFakeProtoWriteMessageBegin
	}
	return nil
}

func (f *fakeProto) WriteMessageEnd() error {
	if f.errOnMessageEnd {
		return errFakeProtoWriteMessageEnd
	}
	return nil
}

func (f *fakeProto) Flush() error {
	if f.errOnFlush {
		return errFakeProtoFlush
	}
	return nil
}

func (f *fakeProto) ReadMessageBegin() (method string, typeID types.MessageType, seqID int32, err error) {
	if f.shouldReturnError {
		err = errFakeProtoReadMessageBegin
		return
	}

	return f.method, f.typeID, f.seqID, nil
}

func (f *fakeResponse) Read(proto types.Decoder) error {
	if f.shouldReturnError {
		return errFakeResponseRead
	}
	return nil
}

func TestSendMsgError(t *testing.T) {
	testCases := []struct {
		proto    Protocol
		request  types.WritableStruct
		expected error
	}{
		// Bad WriteMessageBegin
		{
			proto:    &fakeProto{errOnMessageBegin: true},
			expected: fmt.Errorf("Failed to write message preamble: %w", errFakeProtoWriteMessageBegin),
		},
		// Bad request.Write
		{
			proto:    &fakeProto{errOnMessageBegin: true},
			request:  &fakeRequest{shouldReturnError: true},
			expected: fmt.Errorf("Failed to write message preamble: %w", errFakeProtoWriteMessageBegin),
		},
		// Bad WriteMessageEnd
		{
			proto:    &fakeProto{errOnMessageEnd: true},
			request:  &fakeRequest{shouldReturnError: false},
			expected: fmt.Errorf("Failed to write message epilogue: %w", errFakeProtoWriteMessageEnd),
		},
		// Bad Flush
		{
			proto:    &fakeProto{errOnFlush: true},
			request:  &fakeRequest{shouldReturnError: false},
			expected: errFakeProtoFlush,
		},
	}
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	for i, testCase := range testCases {
		cc := ClientConn{proto: testCase.proto}

		if err := cc.SendMsg(ctx, "foobar", testCase.request, types.CALL); err.Error() != testCase.expected.Error() {
			t.Errorf("#%d: expected call to SendMsg to return \"%+v\"; got \"%+v\"", i, testCase.expected, err)
		}
	}

}

func TestRecvMsgError(t *testing.T) {
	testCases := []struct {
		proto    Protocol
		response types.ReadableStruct
		expected error
	}{
		// Error reading message begin
		{
			proto:    &fakeProto{shouldReturnError: true},
			expected: fmt.Errorf("Failed to read message preamble: %w", errFakeProtoReadMessageBegin),
		},

		// Bad method name in response
		{
			proto:    &fakeProto{method: "foobar2"},
			expected: types.NewApplicationException(types.WRONG_METHOD_NAME, "foobar failed: wrong method name"),
		},

		// Bad seqID in response
		{
			proto:    &fakeProto{method: "foobar", seqID: -1},
			expected: types.NewApplicationException(types.WRONG_METHOD_NAME, "foobar failed: out of sequence response"),
		},

		// Bad typeID in response
		{
			proto:    &fakeProto{method: "foobar", seqID: 0, typeID: -1},
			expected: types.NewApplicationException(types.WRONG_METHOD_NAME, "foobar failed: invalid message type"),
		},

		// Bad REPLY response body read
		{
			proto:    &fakeProto{method: "foobar", seqID: 0, typeID: types.REPLY},
			response: &fakeResponse{shouldReturnError: true},
			expected: fmt.Errorf("Failed to read message body: %w", errFakeResponseRead),
		},
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	for i, testCase := range testCases {
		cc := ClientConn{proto: testCase.proto}

		if err := cc.RecvMsg(ctx, "foobar", testCase.response); err.Error() != testCase.expected.Error() {
			t.Errorf("#%d: expected call to RecvMsg to return \"%+v\"; got \"%+v\"", i, testCase.expected.Error(), err.Error())
		}
	}
}
