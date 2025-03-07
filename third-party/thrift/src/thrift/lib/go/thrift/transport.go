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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

var errTransportInterrupted = errors.New("Transport Interrupted")

// TransportID is the type of the transport, header, rocket, etc.
type TransportID int16

const (
	// TransportIDUnknown is the default value for TransportID
	TransportIDUnknown TransportID = 0
	// TransportIDHeader is the header transport
	// Deprecated: use TransportIDUpgradeToRocket instead.
	TransportIDHeader TransportID = 1
	// TransportIDRocket is the Rocket transport
	TransportIDRocket TransportID = 2
	// TransportIDUpgradeToRocket is the transport that upgrades header to rocket
	TransportIDUpgradeToRocket TransportID = 3
)

func (t TransportID) String() string {
	switch t {
	case TransportIDUnknown:
		return "TransportIDUnknown"
	case TransportIDHeader:
		return "TransportIDHeader"
	case TransportIDRocket:
		return "TransportIDRocket"
	case TransportIDUpgradeToRocket:
		return "TransportIDUpgradeToRocket"
	}
	panic("unreachable")
}

func isEOF(err error) bool {
	if err == nil {
		return false
	}
	if errors.Is(err, io.EOF) {
		return true
	}
	var exp types.TransportException
	if errors.As(err, &exp) && exp.TypeID() == types.END_OF_FILE {
		// connection terminated because client closed connection
		return true
	}
	return false
}
