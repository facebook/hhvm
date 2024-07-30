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
