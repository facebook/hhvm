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
	"iter"
	"net"
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rocket_upgrade"
	"golang.org/x/sync/singleflight"
)

type upgradeToRocketClient struct {
	// Current channel - either 'nil', 'headerProtocol' or 'rocketProtocol',
	// depending on the current state of "upgrade". It will be 'nil' initially
	// to indicate that "upgrade" has not yet been attempted or started.
	actualChannel RequestChannel

	rocketChannel RequestChannel
	headerChannel RequestChannel

	upgradeGroup singleflight.Group
}

var _ RequestChannel = (*upgradeToRocketClient)(nil)

// newUpgradeToRocketClient creates a protocol that upgrades from Header to Rocket client from a socket.
func newUpgradeToRocketClient(
	conn net.Conn,
	protoID types.ProtocolID,
	ioTimeout time.Duration,
	persistentHeaders map[string]string,
) (RequestChannel, error) {
	rocketChannel, err := newRocketClient(conn, protoID, ioTimeout, persistentHeaders)
	if err != nil {
		return nil, err
	}
	headerChannel, err := newHeaderProtocolAsRequestChannel(conn, protoID, ioTimeout, persistentHeaders)
	if err != nil {
		return nil, err
	}

	var actualChannel RequestChannel
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
			actualChannel = rocketChannel
		}
	}

	return &upgradeToRocketClient{
		actualChannel: actualChannel,
		rocketChannel: rocketChannel,
		headerChannel: headerChannel,
	}, nil
}

func (p *upgradeToRocketClient) SendRequestResponse(
	ctx context.Context,
	method string,
	request WritableStruct,
	response ReadableResult,
) error {
	p.maybeUpgrade(ctx)
	return p.actualChannel.SendRequestResponse(ctx, method, request, response)
}

func (p *upgradeToRocketClient) SendRequestNoResponse(
	ctx context.Context,
	method string,
	request WritableStruct,
) error {
	p.maybeUpgrade(ctx)
	return p.actualChannel.SendRequestNoResponse(ctx, method, request)
}

func (p *upgradeToRocketClient) SendRequestStream(
	ctx context.Context,
	method string,
	request WritableStruct,
	response ReadableResult,
	newStreamElemFn func() types.ReadableResult,
) (iter.Seq2[ReadableStruct, error], error) {
	p.maybeUpgrade(ctx)
	return p.actualChannel.SendRequestStream(
		ctx,
		method,
		request,
		response,
		newStreamElemFn,
	)
}

func (p *upgradeToRocketClient) Close() error {
	if p.actualChannel == nil {
		// Upgrade was never atttempted or never succeeded,
		// so we only need to close the 'headerProtocol'.
		return p.headerChannel.Close()
	}
	return p.actualChannel.Close()
}

// maybeUpgrade first sends a upgradeToRocket message using the Header channel.
// If this succeeds, we switch to the Rocket and write the message using it.
// If this fails, we continue using the Header.
func (p *upgradeToRocketClient) maybeUpgrade(ctx context.Context) {
	p.upgradeGroup.Do(
		"upgradeToRocket",
		func() (any, error) {
			if p.actualChannel == nil {
				ruClient := rocket_upgrade.NewRocketUpgradeChannelClient(p.headerChannel)
				err := ruClient.UpgradeToRocket(ctx)
				if err != nil {
					p.actualChannel = p.headerChannel
				} else {
					p.actualChannel = p.rocketChannel
				}
			}
			return nil, nil
		},
	)
}
