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
	"runtime"
	"sync/atomic"

	thriftstats "github.com/facebook/fbthrift/thrift/lib/go/thrift/stats"
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

// This counter is what powers client side load balancing.
// loadFn is a function that reports system load.  It must report the
// server load as an unsigned integer.  Higher numbers mean the server
// is more loaded.  Clients choose the servers that report the lowest
// load.
// NOTE: if you run multiple servers with different capacities, you
// should ensure your load numbers are comparable and account for this
// (i.e. divide by NumCPU)
// NOTE: loadFn is called on every single response.  it should be fast.
func loadFn(stats *thriftstats.ServerStats, totalActiveRequests *atomic.Int64) uint {
	var working int64
	if totalActiveRequests != nil {
		working = totalActiveRequests.Load()
	} else {
		// TODO: remove this once header server is gone
		working = stats.WorkingCount.Get() + stats.SchedulingWorkCount.Get()
	}
	denominator := float64(runtime.NumCPU())
	return uint(1000. * float64(working) / denominator)
}

var loadSheddingError = NewApplicationException(
	LOADSHEDDING,
	"load shedding due to max request limit",
)

var taskExpiredError = NewApplicationException(
	UNKNOWN_APPLICATION_EXCEPTION,
	"Task Expired",
)
