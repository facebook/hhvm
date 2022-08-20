/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

// TransportFactory is used to create wrapped instance of Transports.
// This is used primarily in servers, which get Transports from
// a ServerTransport and then may want to mutate them (i.e. create
// a BufferedTransport from the underlying base transport)
type TransportFactory interface {
	GetTransport(trans Transport) Transport
}

type transportFactory struct{}

// Return a wrapped instance of the base Transport.
func (p *transportFactory) GetTransport(trans Transport) Transport {
	return trans
}

// NewTransportFactory returns a new TransportFactory
func NewTransportFactory() TransportFactory {
	return &transportFactory{}
}
