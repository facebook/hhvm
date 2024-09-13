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

	rsocket "github.com/rsocket/rsocket-go"
	"github.com/rsocket/rsocket-go/payload"
	"github.com/rsocket/rsocket-go/rx"
)

// DO NOT USE: ExperimentalRSocketBlock is a temporary experiment with a new blocking implementation for RSocketClient.
func ExperimentalRSocketBlock() {
	rsocketBlock = func(ctx context.Context, client rsocket.Client, request payload.Payload) (payload.Payload, error) {
		mono := client.RequestResponse(request)
		s := newSubscriber(ctx)
		mono.SubscribeWith(ctx, s)
		return s.Block()
	}
}

type subsriber struct {
	errChan            chan error
	valChan            chan payload.Payload
	completeChan       chan struct{}
	ctx                context.Context
	cancelSubscription func()
}

func newSubscriber(ctx context.Context) *subsriber {
	return &subsriber{
		errChan:      make(chan error, 1),
		valChan:      make(chan payload.Payload, 1),
		completeChan: make(chan struct{}, 1),
		ctx:          ctx,
	}
}

func (s *subsriber) OnNext(msg payload.Payload) {
	s.valChan <- payload.Clone(msg)
}

// OnError represents failed terminal state.
func (s *subsriber) OnError(err error) {
	s.errChan <- err
}

// OnComplete represents successful terminal state.
func (s *subsriber) OnComplete() {
	s.completeChan <- struct{}{}
}

// OnSubscribe invoked after Publisher subscribed.
// No data will start flowing until Subscription#Request is invoked.
func (s *subsriber) OnSubscribe(ctx context.Context, subscription rx.Subscription) {
	select {
	case <-ctx.Done():
		s.OnError(fmt.Errorf("subscriber has been cancelled"))
	default:
		s.cancelSubscription = subscription.Cancel
		subscription.Request(1)
	}
}

func (s *subsriber) Block() (payload.Payload, error) {
	var val payload.Payload
	for {
		select {
		case err := <-s.errChan:
			return val, err
		case val = <-s.valChan:
		case <-s.completeChan:
			return val, nil
		case <-s.ctx.Done():
			return val, s.ctx.Err()
		}
	}
}
