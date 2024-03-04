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
)

// upgradeProtocolFactory is a ProtocolFactory for a protocol that upgrades from Header to Rocket protocol.
type upgradeToRocketProtocolFactory struct {
	headerProtocolFactory ProtocolFactory
	rocketProtocolFactory ProtocolFactory
}

// NewUpgradeToRocketProtocolFactory creates a new upgradeProtocolFactory that upgrades from Header to Rocket protocol.
func NewUpgradeToRocketProtocolFactory() ProtocolFactory {
	return &upgradeToRocketProtocolFactory{
		headerProtocolFactory: NewHeaderProtocolFactory(),
		rocketProtocolFactory: NewRocketProtocolFactory(),
	}
}

func (p *upgradeToRocketProtocolFactory) GetProtocol(trans Transport) Protocol {
	headerProtocol := p.headerProtocolFactory.GetProtocol(trans).(*HeaderProtocol)
	rocketProtocol := p.rocketProtocolFactory.GetProtocol(trans).(*rocketProtocol)
	return NewUpgradeToRocketProtocol(rocketProtocol, headerProtocol)
}

type upgradeToRocketProtocol struct {
	Protocol
	rocketProtocol Protocol
	headerProtocol Protocol
}

// NewUpgradeToRocketProtocol creates a protocol that upgrades from Header to Rocket protocol.
func NewUpgradeToRocketProtocol(rocketProtocol Protocol, headerProtocol Protocol) Protocol {
	return &upgradeToRocketProtocol{
		rocketProtocol: rocketProtocol,
		headerProtocol: headerProtocol,
	}
}

// WriteMessageBegin first sends a upgradeToRocket message using the HeaderProtocol.
// If this succeeds, we switch to the RocketProtocol and write the message using it.
// If this fails, we send the original message using the HeaderProtocol and continue using the HeaderProtocol.
func (p *upgradeToRocketProtocol) WriteMessageBegin(name string, typeID MessageType, seqid int32) error {
	if p.Protocol == nil {
		if err := p.upgradeToRocket(); err != nil {
			p.Protocol = p.headerProtocol
		} else {
			p.Protocol = p.rocketProtocol
		}
	}
	return p.Protocol.WriteMessageBegin(name, typeID, seqid)
}

func (p *upgradeToRocketProtocol) upgradeToRocket() error {
	return upgradeToRocket(context.Background(), p.headerProtocol)
}

func (p *upgradeToRocketProtocol) SetPersistentHeader(key, value string) {
	if p.Protocol == nil {
		p.rocketProtocol.(PersistentHeaders).SetPersistentHeader(key, value)
		p.headerProtocol.(PersistentHeaders).SetPersistentHeader(key, value)
		return
	}
	p.Protocol.(PersistentHeaders).SetPersistentHeader(key, value)
}

func (p *upgradeToRocketProtocol) GetPersistentHeader(key string) (string, bool) {
	v, ok := p.GetPersistentHeaders()[key]
	return v, ok
}

func (p *upgradeToRocketProtocol) GetPersistentHeaders() map[string]string {
	if p.Protocol == nil {
		headers := p.headerProtocol.(PersistentHeaders).GetPersistentHeaders()
		rocketHeaders := p.rocketProtocol.(PersistentHeaders).GetPersistentHeaders()
		for k, v := range rocketHeaders {
			headers[k] = v
		}
		return headers
	}
	return p.Protocol.(PersistentHeaders).GetPersistentHeaders()
}

func (p *upgradeToRocketProtocol) ClearPersistentHeaders() {
	if p.Protocol == nil {
		p.rocketProtocol.(PersistentHeaders).ClearPersistentHeaders()
		p.headerProtocol.(PersistentHeaders).ClearPersistentHeaders()
		return
	}
	p.Protocol.(PersistentHeaders).ClearPersistentHeaders()
}

func (p *upgradeToRocketProtocol) setRequestHeader(key, value string) {
	if p.Protocol == nil {
		p.rocketProtocol.(requestHeaders).setRequestHeader(key, value)
		p.headerProtocol.(requestHeaders).setRequestHeader(key, value)
		return
	}
	p.Protocol.(requestHeaders).setRequestHeader(key, value)
}

func (p *upgradeToRocketProtocol) getRequestHeaders() map[string]string {
	if p.Protocol == nil {
		headers := p.headerProtocol.(requestHeaders).getRequestHeaders()
		rocketHeaders := p.rocketProtocol.(requestHeaders).getRequestHeaders()
		for k, v := range rocketHeaders {
			headers[k] = v
		}
		return headers
	}
	return p.Protocol.(requestHeaders).getRequestHeaders()
}
