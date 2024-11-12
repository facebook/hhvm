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
	"crypto/tls"
	"net"
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

type upgradeToRocketClient struct {
	// Current protocol - either 'nil', 'headerProtocol' or 'rocketProtocol',
	// depending on the current state of "upgrade". It will be 'nil' initially
	// to indicate that "upgrade" has not yet been attempted or started.
	types.Protocol

	rocketProtocol types.Protocol
	headerProtocol types.Protocol
}

var _ types.Protocol = (*upgradeToRocketClient)(nil)
var _ types.RequestHeaders = (*upgradeToRocketClient)(nil)
var _ types.ResponseHeaderGetter = (*upgradeToRocketClient)(nil)

// newUpgradeToRocketClient creates a protocol that upgrades from Header to Rocket client from a socket.
func newUpgradeToRocketClient(conn net.Conn, protoID types.ProtocolID, timeout time.Duration, persistentHeaders map[string]string) (types.Protocol, error) {
	rocket, err := newRocketClient(conn, protoID, timeout, persistentHeaders)
	if err != nil {
		return nil, err
	}
	header, err := newHeaderProtocol(conn, protoID, timeout, persistentHeaders)
	if err != nil {
		return nil, err
	}

	var protocol Protocol
	// Explicitly force TLS handshake protocol to run (if this is a TLS connection).
	//
	// Usually, TLS handshake is done implicitly/seamlessly by 'crypto/tls' package,
	// whenever Read/Write functions are invoked on a connection for the first time.
	// However, in our case, we require the handshake to be complete ahead of any
	// Read/Write calls - so that we can access ALPN value and choose the transport.
	tlsConn, isTLS := conn.(*tls.Conn)
	if isTLS {
		err := tlsConn.Handshake()
		if err != nil {
			return nil, err
		}
		tlsConnState := tlsConn.ConnectionState()
		// Use Rocket protocol right away if ALPN value is set to "rs".
		if tlsConnState.NegotiatedProtocol == "rs" {
			protocol = rocket
		}
	}

	return &upgradeToRocketClient{
		Protocol:       protocol,
		rocketProtocol: rocket,
		headerProtocol: header,
	}, nil
}

// WriteMessageBegin first sends a upgradeToRocket message using the HeaderProtocol.
// If this succeeds, we switch to the RocketProtocol and write the message using it.
// If this fails, we send the original message using the HeaderProtocol and continue using the HeaderProtocol.
func (p *upgradeToRocketClient) WriteMessageBegin(name string, typeID types.MessageType, seqid int32) error {
	if p.Protocol == nil {
		if err := p.upgradeToRocket(); err != nil {
			p.Protocol = p.headerProtocol
		} else {
			p.Protocol = p.rocketProtocol
		}
	}
	return p.Protocol.WriteMessageBegin(name, typeID, seqid)
}

func (p *upgradeToRocketClient) upgradeToRocket() error {
	return upgradeToRocket(context.Background(), p.headerProtocol)
}

func (p *upgradeToRocketClient) SetRequestHeader(key, value string) {
	if p.Protocol == nil {
		p.rocketProtocol.(types.RequestHeaders).SetRequestHeader(key, value)
		p.headerProtocol.(types.RequestHeaders).SetRequestHeader(key, value)
		return
	}
	p.Protocol.(types.RequestHeaders).SetRequestHeader(key, value)
}

func (p *upgradeToRocketClient) GetResponseHeaders() map[string]string {
	if p.Protocol == nil {
		headers := p.headerProtocol.GetResponseHeaders()
		rocketHeaders := p.rocketProtocol.GetResponseHeaders()
		for k, v := range rocketHeaders {
			headers[k] = v
		}
		return headers
	}
	return p.Protocol.GetResponseHeaders()
}

func (p *upgradeToRocketClient) Close() error {
	if p.Protocol == nil {
		// Upgrade was never atttempted or never succeeded,
		// so we only need to close the 'headerProtocol'.
		return p.headerProtocol.Close()
	}
	return p.Protocol.Close()
}
