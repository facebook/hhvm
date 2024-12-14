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
	"sync"

	rsocket "github.com/rsocket/rsocket-go"
	"github.com/rsocket/rsocket-go/payload"
	"github.com/rsocket/rsocket-go/rx"
)

func rsocketBlock(ctx context.Context, client rsocket.Client, request payload.Payload) (payload.Payload, error) {
	mono := client.RequestResponse(request)
	// This implementation of subscriber avoids race conditions on close that is present in the default implementation in the rsocket library.
	sub := newSubscriber()
	mono.SubscribeWith(ctx, sub)
	return sub.Block(ctx)
}

type subsriber struct {
	// Error and Complete are two possible terminal states.
	errChan            chan error
	completeChan       chan struct{}
	payload            payload.Payload
	payloadMutex       sync.Mutex
	cancelSubscription func()
}

func newSubscriber() *subsriber {
	return &subsriber{
		errChan:      make(chan error, 1),
		completeChan: make(chan struct{}, 1),
	}
}

func (s *subsriber) OnNext(msg payload.Payload) {
	s.payloadMutex.Lock()
	defer s.payloadMutex.Unlock()
	s.payload = payload.Clone(msg)
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

func (s *subsriber) Block(ctx context.Context) (payload.Payload, error) {
	for {
		select {
		case err := <-s.errChan:
			return nil, err
		case <-s.completeChan:
			s.payloadMutex.Lock()
			defer s.payloadMutex.Unlock()
			return s.payload, nil
		case <-ctx.Done():
			return nil, ctx.Err()
		}
	}
}
