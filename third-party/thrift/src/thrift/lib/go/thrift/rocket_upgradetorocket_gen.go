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
)

// This was generated by Thrift from RocketUpgrade.thrift
// It is temporarily copied to implement rocket and
// to avoid the conflicting imports between go build and buck.
// This also avoids the circular dependency issue.
// This file is probably not the place you want to edit!

// upgradeToRocket sends an upgradeToRocket service call to the server and returns any errors.
// the protocol parameter has to be a HeaderProtocol.
func upgradeToRocket(_ context.Context, protocol Protocol) error {
	method := "upgradeToRocket"

	// send upgradeToRocket service call
	request := &reqServiceUpgradeToRocket{}
	seqID := int32(1)
	if err := protocol.WriteMessageBegin(method, CALL, seqID); err != nil {
		return err
	}
	if err := request.Write(protocol); err != nil {
		return err
	}
	if err := protocol.WriteMessageEnd(); err != nil {
		return err
	}
	if err := protocol.Flush(); err != nil {
		return err
	}

	// receive upgradeToRocket service response, which is void
	response := &respServiceUpgradeToRocket{}
	recvMethod, mTypeID, _, err := protocol.ReadMessageBegin()
	if err != nil {
		return err
	}
	if method != recvMethod {
		return NewApplicationException(WRONG_METHOD_NAME, fmt.Sprintf("%s failed: wrong method name", method))
	}
	switch mTypeID {
	case REPLY:
		if err := response.Read(protocol); err != nil {
			return err
		}

		return protocol.ReadMessageEnd()
	case EXCEPTION:
		appException := NewApplicationException(UNKNOWN_APPLICATION_EXCEPTION, "Unknown exception")
		if err := appException.Read(protocol); err != nil {
			return err
		}
		if err := protocol.ReadMessageEnd(); err != nil {
			return err
		}
		return appException
	default:
		return NewApplicationException(INVALID_MESSAGE_TYPE_EXCEPTION, fmt.Sprintf("%s failed: invalid message type", method))
	}
}

type reqServiceUpgradeToRocket struct{}

func (x *reqServiceUpgradeToRocket) Write(p Encoder) error {
	if err := p.WriteStructBegin("reqServiceUpgradeToRocket"); err != nil {
		return PrependError(fmt.Sprintf("%T write struct begin error: ", x), err)
	}

	if err := p.WriteFieldStop(); err != nil {
		return PrependError(fmt.Sprintf("%T write field stop error: ", x), err)
	}

	if err := p.WriteStructEnd(); err != nil {
		return PrependError(fmt.Sprintf("%T write struct end error: ", x), err)
	}
	return nil
}

func (x *reqServiceUpgradeToRocket) Read(p Decoder) error {
	if _, err := p.ReadStructBegin(); err != nil {
		return PrependError(fmt.Sprintf("%T read error: ", x), err)
	}

	for {
		_, wireType, id, err := p.ReadFieldBegin()
		if err != nil {
			return PrependError(fmt.Sprintf("%T field %d read error: ", x, id), err)
		}

		if wireType == STOP {
			break
		}

		switch {
		default:
			if err := p.Skip(wireType); err != nil {
				return err
			}
		}

		if err := p.ReadFieldEnd(); err != nil {
			return err
		}
	}

	if err := p.ReadStructEnd(); err != nil {
		return PrependError(fmt.Sprintf("%T read struct end error: ", x), err)
	}

	return nil
}

type respServiceUpgradeToRocket struct{}

func (x *respServiceUpgradeToRocket) Write(p Encoder) error {
	if err := p.WriteStructBegin("respServiceUpgradeToRocket"); err != nil {
		return PrependError(fmt.Sprintf("%T write struct begin error: ", x), err)
	}

	if err := p.WriteFieldStop(); err != nil {
		return PrependError(fmt.Sprintf("%T write field stop error: ", x), err)
	}

	if err := p.WriteStructEnd(); err != nil {
		return PrependError(fmt.Sprintf("%T write struct end error: ", x), err)
	}
	return nil
}

func (x *respServiceUpgradeToRocket) Read(p Decoder) error {
	if _, err := p.ReadStructBegin(); err != nil {
		return PrependError(fmt.Sprintf("%T read error: ", x), err)
	}

	for {
		_, wireType, id, err := p.ReadFieldBegin()
		if err != nil {
			return PrependError(fmt.Sprintf("%T field %d read error: ", x, id), err)
		}

		if wireType == STOP {
			break
		}

		switch {
		default:
			if err := p.Skip(wireType); err != nil {
				return err
			}
		}

		if err := p.ReadFieldEnd(); err != nil {
			return err
		}
	}

	if err := p.ReadStructEnd(); err != nil {
		return PrependError(fmt.Sprintf("%T read struct end error: ", x), err)
	}

	return nil
}
