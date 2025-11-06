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

package dummy

import (
	"context"
	"errors"
	"time"

	"github.com/facebook/fbthrift/thrift/test/go/if/dummy"
)

type DummyHandler struct {
	// Testing helper to verify that oneway RPCs get delivered
	OnewayRPCRequests chan<- string
}

// Compile time interface enforcer
var _ dummy.Dummy = (*DummyHandler)(nil)

func (h *DummyHandler) Ping(_ context.Context) error {
	return nil
}

func (h *DummyHandler) Echo(_ context.Context, value string) (string, error) {
	return value, nil
}

func (h *DummyHandler) OnewayRPC(_ context.Context, value string) error {
	if h.OnewayRPCRequests != nil {
		h.OnewayRPCRequests <- value
	}
	return nil
}

func (h *DummyHandler) Sleep(_ context.Context, milliseconds int64) error {
	time.Sleep(time.Duration(milliseconds) * time.Millisecond)
	return nil
}

func (h *DummyHandler) Panic(_ context.Context) error {
	panic("panic!")
}

func (h *DummyHandler) GetDeclaredException(_ context.Context) error {
	return dummy.NewDummyException().SetMessage("hello")
}

func (h *DummyHandler) GetUndeclaredException(_ context.Context) error {
	return errors.New("undeclared exception")
}

func (h *DummyHandler) StreamOnly(_ context.Context, from int32, to int32) (func(context.Context, chan<- int32) error, error) {
	if from >= to {
		return nil, errors.New("'from' is greater than or equal to 'to'")
	}

	elemProducerFunc := func(ctx context.Context, elemChan chan<- int32) error {
		for i := from; i < to; i++ {
			select {
			case <-ctx.Done():
				return ctx.Err()
			case elemChan <- i:
			}
		}
		return nil
	}

	return elemProducerFunc, nil
}

func (h *DummyHandler) ResponseAndStream(_ context.Context, from int32, to int32) (int32, func(context.Context, chan<- int32) error, error) {
	if from >= to {
		return 0, nil, errors.New("'from' is greater than or equal to 'to'")
	}

	elemProducerFunc := func(ctx context.Context, elemChan chan<- int32) error {
		for i := from; i < to; i++ {
			select {
			case <-ctx.Done():
				return ctx.Err()
			case elemChan <- i:
			}
		}
		return nil
	}

	return to - from, elemProducerFunc, nil
}

func (h *DummyHandler) StreamWithDeclaredException(_ context.Context) (func(context.Context, chan<- int32) error, error) {
	elemProducerFunc := func(_ context.Context, elemChan chan<- int32) error {
		return dummy.NewDummyException().SetMessage("hello")
	}
	return elemProducerFunc, nil
}

func (h *DummyHandler) StreamWithUndeclaredException(_ context.Context) (func(context.Context, chan<- int32) error, error) {
	elemProducerFunc := func(_ context.Context, elemChan chan<- int32) error {
		return errors.New("undeclared exception")
	}
	return elemProducerFunc, nil
}

func (h *DummyHandler) ResponseAndStreamWithDeclaredException(_ context.Context) (int32, func(context.Context, chan<- int32) error, error) {
	elemProducerFunc := func(_ context.Context, elemChan chan<- int32) error {
		return nil
	}
	return 0, elemProducerFunc, dummy.NewDummyException().SetMessage("hello")
}

func (h *DummyHandler) ResponseAndStreamWithUndeclaredException(_ context.Context) (int32, func(context.Context, chan<- int32) error, error) {
	elemProducerFunc := func(_ context.Context, elemChan chan<- int32) error {
		return nil
	}
	return 0, elemProducerFunc, errors.New("undeclared exception")
}

func (h *DummyHandler) CreateSummer(_ context.Context) (*dummy.SummerProcessor, error) {
	return dummy.NewSummerProcessor(&SummerHandler{}), nil
}

type SummerHandler struct {
	sum int32
}

func (h *SummerHandler) Add(_ context.Context, val int32) (int32, error) {
	h.sum += val
	return h.sum, nil
}
