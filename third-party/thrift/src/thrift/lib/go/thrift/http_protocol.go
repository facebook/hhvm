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
	"strings"
	"time"
)

type httpProtocol struct {
	Format
	transport         *httpClient
	protoID           ProtocolID
	persistentHeaders map[string]string
}

// NewHTTPProtocol creates a Protocol from a format that serializes directly to an HTTPClient.
func NewHTTPProtocol(url string) (Protocol, error) {
	httpClient, err := newHTTPPostClient(url)
	if err != nil {
		return nil, err
	}
	p := &httpProtocol{
		transport:         httpClient,
		persistentHeaders: make(map[string]string),
		protoID:           ProtocolIDCompact,
	}
	p.SetRequestHeader("User-Agent", "Go/THttpClient")
	if err := p.resetProtocol(); err != nil {
		return nil, err
	}
	return p, nil
}

func (p *httpProtocol) SetTimeout(timeout time.Duration) {
	p.transport.client.Timeout = timeout
}

func (p *httpProtocol) resetProtocol() error {
	switch p.protoID {
	case ProtocolIDBinary:
		// These defaults match cpp implementation
		p.Format = NewBinaryProtocol(p.transport, false, true)
	case ProtocolIDCompact:
		p.Format = NewCompactProtocol(p.transport)
	default:
		return NewProtocolException(fmt.Errorf("Unknown protocol id: %#x", p.protoID))
	}
	return nil
}

func (p *httpProtocol) SetProtocolID(protoID ProtocolID) error {
	p.protoID = protoID
	return p.resetProtocol()
}

func (p *httpProtocol) Close() error {
	return p.transport.Close()
}

func (p *httpProtocol) SetPersistentHeader(key, value string) {
	p.persistentHeaders[key] = value
}

func (p *httpProtocol) GetPersistentHeader(key string) (string, bool) {
	v, ok := p.persistentHeaders[key]
	return v, ok
}

func (p *httpProtocol) GetResponseHeader(key string) (string, bool) {
	return "", false
}

func (p *httpProtocol) GetResponseHeaders() map[string]string {
	return nil
}

func (p *httpProtocol) SetRequestHeader(key, value string) {
	p.transport.SetHeader(key, value)
}

func (p *httpProtocol) GetRequestHeaders() map[string]string {
	headers := make(map[string]string)
	for k, v := range p.transport.header {
		headers[k] = strings.Join(v, ",")
	}
	return headers
}

func (p *httpProtocol) WriteMessageBegin(name string, typeID MessageType, seqid int32) error {
	for k, v := range p.persistentHeaders {
		p.transport.SetHeader(k, v)
	}
	return p.Format.WriteMessageBegin(name, typeID, seqid)
}
