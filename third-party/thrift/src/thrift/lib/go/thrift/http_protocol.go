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
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/format"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

type httpProtocol struct {
	types.Format
	transport         *httpClient
	protoID           types.ProtocolID
	persistentHeaders map[string]string
}

var _ Protocol = (*httpProtocol)(nil)

// NewHTTPRequestChannel creates a Protocol from a format that serializes directly to an HTTPClient.
func NewHTTPRequestChannel(url string) (RequestChannel, error) {
	httpClient, err := newHTTPPostClient(url)
	if err != nil {
		return nil, err
	}
	p := &httpProtocol{
		transport:         httpClient,
		persistentHeaders: make(map[string]string),
		protoID:           types.ProtocolIDCompact,
	}
	p.setRequestHeader("User-Agent", "Go/THttpClient")
	if err := p.resetProtocol(); err != nil {
		return nil, err
	}
	return NewSerialChannel(p), nil
}

func (p *httpProtocol) SetTimeout(timeout time.Duration) {
	p.transport.client.Timeout = timeout
}

func (p *httpProtocol) resetProtocol() error {
	switch p.protoID {
	case types.ProtocolIDBinary:
		// These defaults match cpp implementation
		p.Format = format.NewBinaryFormatOptions(p.transport, false, true)
	case types.ProtocolIDCompact:
		p.Format = format.NewCompactFormat(p.transport)
	default:
		return types.NewProtocolException(fmt.Errorf("Unknown protocol id: %d", p.protoID))
	}
	return nil
}

func (p *httpProtocol) SetProtocolID(protoID types.ProtocolID) error {
	p.protoID = protoID
	return p.resetProtocol()
}

func (p *httpProtocol) Close() error {
	return p.transport.Close()
}

func (p *httpProtocol) getResponseHeaders() map[string]string {
	return nil
}

func (p *httpProtocol) setRequestHeader(key, value string) {
	p.transport.SetHeader(key, value)
}

func (p *httpProtocol) WriteMessageBegin(name string, typeID types.MessageType, seqid int32) error {
	for k, v := range p.persistentHeaders {
		p.transport.SetHeader(k, v)
	}
	return p.Format.WriteMessageBegin(name, typeID, seqid)
}
