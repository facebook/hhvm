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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

type rocketUpgradeProcessor struct {
	upgraded  bool
	processor types.Processor
}

func newRocketUpgradeProcessor(processor types.Processor) *rocketUpgradeProcessor {
	return &rocketUpgradeProcessor{processor: processor}
}

func (r *rocketUpgradeProcessor) GetProcessorFunction(name string) types.ProcessorFunction {
	// The upgradeToRocket function in thrift/lib/thrift/RocketUpgrade.thrift
	// asks the server to upgrade to using the rocket protocol.
	// If the server does not respond with an error,
	// it is assumed that the server has upgraded to rocket.
	if name == "upgradeToRocket" {
		r.upgraded = true
		return &rocketUpgradeProcessorFunction{}
	}
	return r.processor.GetProcessorFunction(name)
}

type rocketUpgradeProcessorFunction struct{}

func (r *rocketUpgradeProcessorFunction) Read(prot types.Decoder) (types.Struct, types.Exception) {
	args := &reqServiceUpgradeToRocket{}
	if err := args.Read(prot); err != nil {
		return nil, err
	}
	return args, prot.ReadMessageEnd()
}

func (r *rocketUpgradeProcessorFunction) RunContext(_ context.Context, _ types.Struct) (types.WritableStruct, types.ApplicationException) {
	return &respServiceUpgradeToRocket{}, nil
}

func (r *rocketUpgradeProcessorFunction) Write(seqID int32, result types.WritableStruct, prot types.Encoder) (err types.Exception) {
	var err2 error
	messageType := types.REPLY
	switch result.(type) {
	case types.ApplicationException:
		messageType = types.EXCEPTION
	}

	if err2 = prot.WriteMessageBegin("upgradeToRocket", messageType, seqID); err2 != nil {
		err = err2
	}
	if err2 = result.Write(prot); err == nil && err2 != nil {
		err = err2
	}
	if err2 = prot.WriteMessageEnd(); err == nil && err2 != nil {
		err = err2
	}
	if err2 = prot.Flush(); err == nil && err2 != nil {
		err = err2
	}
	return err
}
