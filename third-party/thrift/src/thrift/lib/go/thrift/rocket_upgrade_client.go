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
	"net"
	"time"
)

type upgradeToRocketClient struct {
	Protocol
	rocketProtocol Protocol
	headerProtocol Protocol
}

// newUpgradeToRocketClient creates a protocol that upgrades from Header to Rocket client from a socket.
func newUpgradeToRocketClient(conn net.Conn, protoID ProtocolID, timeout time.Duration, persistentHeaders map[string]string) (Protocol, error) {
	rocket, err := newRocketClient(conn, protoID, timeout, persistentHeaders)
	if err != nil {
		return nil, err
	}
	header, err := newHeaderProtocol(conn, protoID, timeout, persistentHeaders)
	if err != nil {
		return nil, err
	}
	return &upgradeToRocketClient{
		rocketProtocol: rocket,
		headerProtocol: header,
	}, nil
}

// WriteMessageBegin first sends a upgradeToRocket message using the HeaderProtocol.
// If this succeeds, we switch to the RocketProtocol and write the message using it.
// If this fails, we send the original message using the HeaderProtocol and continue using the HeaderProtocol.
func (p *upgradeToRocketClient) WriteMessageBegin(name string, typeID MessageType, seqid int32) error {
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
		p.rocketProtocol.(RequestHeaders).SetRequestHeader(key, value)
		p.headerProtocol.(RequestHeaders).SetRequestHeader(key, value)
		return
	}
	p.Protocol.(RequestHeaders).SetRequestHeader(key, value)
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
		if err := p.headerProtocol.Close(); err != nil {
			return err
		}
		return p.rocketProtocol.Close()
	}
	return p.Protocol.Close()
}
