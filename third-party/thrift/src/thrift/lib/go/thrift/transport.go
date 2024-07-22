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
	"errors"
	"io"
)

var errTransportInterrupted = errors.New("Transport Interrupted")

// TransportID is the type of the transport, header, rocket, etc.
type TransportID int16

const (
	// TransportIDUnknown is the default value for TransportID
	TransportIDUnknown TransportID = 0
	// TransportIDHeader is the header transport
	TransportIDHeader TransportID = 1
	// Deprecated: use TransportIDUpgradeToRocket, this is only used for testing purposes.
	TransportIDRocket TransportID = 2
	// TransportIDUpgradeToRocket is the transport that upgrades header to rocket
	TransportIDUpgradeToRocket TransportID = 3
)

// Flusher is the interface that wraps the basic Flush method
type Flusher interface {
	Flush() (err error)
}

// ReadSizeProvider is the interface that wraps the basic RemainingBytes method
type ReadSizeProvider interface {
	RemainingBytes() (numBytes uint64)
}

type stringWriter interface {
	WriteString(s string) (n int, err error)
}

// RichTransport is an "enhanced" transport with extra capabilities.
// You need to use one of these to construct protocol.
// Notably, Socket does not implement this interface, and it is always a mistake to use
// Socket directly in protocol.
type RichTransport interface {
	io.ReadWriteCloser
	io.ByteReader
	io.ByteWriter
	stringWriter
	Flusher
	ReadSizeProvider
}

// UnknownRemaining is used by transports that can not return a real answer
// for RemainingBytes()
const UnknownRemaining = ^uint64(0)
