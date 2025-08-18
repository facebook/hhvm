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
	"github.com/facebook/fbthrift/thrift/lib/thrift/rocket_upgrade"
)

type upgradeToRocketClient struct {
	// Current protocol - either 'nil', 'headerProtocol' or 'rocketProtocol',
	// depending on the current state of "upgrade". It will be 'nil' initially
	// to indicate that "upgrade" has not yet been attempted or started.
	Protocol

	rocketProtocol Protocol
	headerProtocol Protocol
}

var _ Protocol = (*upgradeToRocketClient)(nil)

// newUpgradeToRocketClient creates a protocol that upgrades from Header to Rocket client from a socket.
func newUpgradeToRocketClient(
	conn net.Conn,
	protoID types.ProtocolID,
	ioTimeout time.Duration,
	persistentHeaders map[string]string,
) (Protocol, error) {
	rocket, err := newRocketClient(conn, protoID, ioTimeout, persistentHeaders)
	if err != nil {
		return nil, err
	}
	header, err := newHeaderProtocol(conn, protoID, ioTimeout, persistentHeaders)
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
		ruClient := rocket_upgrade.NewRocketUpgradeChannelClient(NewSerialChannel(p.headerProtocol))
		err := ruClient.UpgradeToRocket(context.Background())
		if err != nil {
			p.Protocol = p.headerProtocol
		} else {
			p.Protocol = p.rocketProtocol
		}
	}
	return p.Protocol.WriteMessageBegin(name, typeID, seqid)
}

func (p *upgradeToRocketClient) setRequestHeader(key, value string) {
	if p.Protocol == nil {
		p.rocketProtocol.setRequestHeader(key, value)
		p.headerProtocol.setRequestHeader(key, value)
		return
	}
	p.Protocol.setRequestHeader(key, value)
}

func (p *upgradeToRocketClient) getResponseHeaders() map[string]string {
	if p.Protocol == nil {
		headers := p.headerProtocol.getResponseHeaders()
		rocketHeaders := p.rocketProtocol.getResponseHeaders()
		for k, v := range rocketHeaders {
			headers[k] = v
		}
		return headers
	}
	return p.Protocol.getResponseHeaders()
}

func (p *upgradeToRocketClient) Close() error {
	if p.Protocol == nil {
		// Upgrade was never atttempted or never succeeded,
		// so we only need to close the 'headerProtocol'.
		return p.headerProtocol.Close()
	}
	return p.Protocol.Close()
}
