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
	"fmt"
)

// protocolBuffer is a protocol that simply buffers all calls:
// - saves the bytes in the write buffer
// - reads from the read buffer
// - saves request headers
// - returns response headers
type protocolBuffer struct {
	Decoder
	Encoder
	wbuf        *MemoryBuffer
	rbuf        *MemoryBuffer
	name        string
	messageType MessageType
	seqID       int32
	reqHeaders  map[string]string
	respHeaders map[string]string
}

var _ Protocol = (*protocolBuffer)(nil)

func newProtocolBuffer(respHeaders map[string]string, protoID ProtocolID, data []byte) (*protocolBuffer, error) {
	p := &protocolBuffer{
		respHeaders: respHeaders,
		reqHeaders:  map[string]string{},
		wbuf:        NewMemoryBuffer(),
		rbuf:        NewMemoryBufferWithData(data),
	}
	switch protoID {
	case ProtocolIDBinary:
		p.Decoder = newBinaryDecoder(p.rbuf)
		p.Encoder = newBinaryEncoder(p.wbuf)
	case ProtocolIDCompact:
		p.Decoder = newCompactDecoder(p.rbuf)
		p.Encoder = newCompactEncoder(p.wbuf)
	default:
		return nil, NewProtocolException(fmt.Errorf("Unknown protocol id: %#x", protoID))
	}
	return p, nil
}

func (b *protocolBuffer) Bytes() []byte {
	return b.wbuf.Bytes()
}

func (b *protocolBuffer) ReadMessageBegin() (string, MessageType, int32, error) {
	return b.name, b.messageType, 0, nil
}

func (b *protocolBuffer) ReadMessageEnd() error {
	return nil
}

func (b *protocolBuffer) WriteMessageBegin(name string, messageType MessageType, seqid int32) error {
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

func (b *protocolBuffer) GetResponseHeaders() map[string]string {
	return b.respHeaders
}

func (b *protocolBuffer) SetRequestHeader(key, value string) {
	b.reqHeaders[key] = value
}

func (b *protocolBuffer) getRequestHeaders() map[string]string {
	return b.reqHeaders
}
