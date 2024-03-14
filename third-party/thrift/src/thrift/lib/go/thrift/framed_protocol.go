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

type framedProtocol struct {
	Format
	transport *FramedTransport
}

// NewFramedProtocol creates a Protocol from a format that serializes directly to an FramedTransport.
func NewFramedProtocol(transport *FramedTransport, format Format) Protocol {
	return &framedProtocol{
		Format:    format,
		transport: transport,
	}
}

func (p *framedProtocol) SetPersistentHeader(key, value string) {}

func (p *framedProtocol) GetPersistentHeader(key string) (string, bool) {
	return "", false
}

func (p *framedProtocol) GetPersistentHeaders() map[string]string {
	return nil
}

func (p *framedProtocol) ClearPersistentHeaders() {}

func (p *framedProtocol) GetResponseHeader(key string) (string, bool) {
	return "", false
}

func (p *framedProtocol) GetResponseHeaders() map[string]string {
	return nil
}

func (p *framedProtocol) SetRequestHeader(key, value string) {}

func (p *framedProtocol) GetRequestHeaders() map[string]string {
	return nil
}

func (p *framedProtocol) Close() error {
	return p.transport.Close()
}
