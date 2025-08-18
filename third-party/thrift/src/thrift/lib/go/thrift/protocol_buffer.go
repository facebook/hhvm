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
	"fmt"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/format"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

// protocolBuffer is a protocol that simply buffers all calls:
// - saves the bytes in the write buffer
// - reads from the read buffer
// - saves request headers
// - returns response headers
type protocolBuffer struct {
	types.Decoder
	types.Encoder
	wbuf        *bytes.Buffer
	rbuf        *bytes.Buffer
	name        string
	messageType types.MessageType
	seqID       int32
	reqHeaders  map[string]string
	respHeaders map[string]string
}

var _ Protocol = (*protocolBuffer)(nil)

func newProtocolBuffer(respHeaders map[string]string, protoID types.ProtocolID, data []byte) (*protocolBuffer, error) {
	p := &protocolBuffer{
		respHeaders: respHeaders,
		reqHeaders:  map[string]string{},
		wbuf:        new(bytes.Buffer),
		rbuf:        bytes.NewBuffer(data),
	}
	switch protoID {
	case types.ProtocolIDBinary:
		p.Decoder = format.NewBinaryDecoder(p.rbuf)
		p.Encoder = format.NewBinaryEncoder(p.wbuf)
	case types.ProtocolIDCompact:
		p.Decoder = format.NewCompactDecoder(p.rbuf)
		p.Encoder = format.NewCompactEncoder(p.wbuf)
	default:
		return nil, types.NewProtocolException(fmt.Errorf("Unknown protocol id: %d", protoID))
	}
	return p, nil
}

func (b *protocolBuffer) Bytes() []byte {
	return b.wbuf.Bytes()
}

func (b *protocolBuffer) ReadMessageBegin() (string, types.MessageType, int32, error) {
	return b.name, b.messageType, 0, nil
}

func (b *protocolBuffer) ReadMessageEnd() error {
	return nil
}

func (b *protocolBuffer) WriteMessageBegin(name string, messageType types.MessageType, seqid int32) error {
	b.name = name
	b.messageType = messageType
	b.seqID = seqid
	return nil
}

func (b *protocolBuffer) WriteMessageEnd() error {
	return nil
}

func (b *protocolBuffer) Close() error {
	return nil
}

func (b *protocolBuffer) getResponseHeaders() map[string]string {
	return b.respHeaders
}

func (b *protocolBuffer) setRequestHeader(key, value string) {
	b.reqHeaders[key] = value
}

func (b *protocolBuffer) getRequestHeaders() map[string]string {
	return b.reqHeaders
}
