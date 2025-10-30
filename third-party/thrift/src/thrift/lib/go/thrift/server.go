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
	"fmt"
	"net"
)

// Server is a thrift server
type Server interface {
	// ServeContext starts the server, and stops it when the context is cancelled
	ServeContext(ctx context.Context) error
}

// NewServer creates a new thrift server. It includes:
// * load shedding support
// * load balancing compatible with high QPS services
// * pipelining of incoming requests on same connection
// * out of order responses (for clients that support it!)
// * and statstics that you can export to your favorite monitoring system
func NewServer(processor Processor, listener net.Listener, transportType TransportID, options ...ServerOption) Server {
	serverOptions := newServerOptions(options...)
	switch transportType {
	case TransportIDHeader:
		// NOTE: temporary workaround while Header support is being removed.
		// This code is never hit actually.
		return newUpgradeToRocketServer(processor, listener, serverOptions)
	case TransportIDRocket:
		return newRocketServer(processor, listener, serverOptions)
	case TransportIDUpgradeToRocket:
		return newUpgradeToRocketServer(processor, listener, serverOptions)
	default:
		panic(fmt.Sprintf("Server does not support: %v", transportType))
	}
}

var loadSheddingError = NewApplicationException(
	LOADSHEDDING,
	"load shedding due to max request limit",
)

var taskExpiredError = NewApplicationException(
	UNKNOWN_APPLICATION_EXCEPTION,
	"Task Expired",
)
