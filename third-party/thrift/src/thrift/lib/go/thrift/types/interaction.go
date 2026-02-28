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

package types

import (
	"context"
)

type interactionContextKey int

const (
	interactionCreateKey interactionContextKey = iota + 1
	interactionIDKey
	interactionServerCreateKey
)

// Terminable is implemented by an interaction processor.
type Terminable interface {
	OnTermination()
}

// Note: Processor propagation via context is a temporary workaround.
// It's not very idiomatic or clean, but it allows us to support
// interactions a lot easier than if we had to change the codegen API.

func WithInteractionCreateContext(ctx context.Context) context.Context {
	value := map[int64]any{}
	return context.WithValue(ctx, interactionServerCreateKey, value)
}

func GetInteractionCreateProcessor(ctx context.Context) any {
	value := ctx.Value(interactionServerCreateKey).(map[int64]any)
	return value[0]
}

func SetInteractionCreateProcessor(ctx context.Context, proc any) {
	value := ctx.Value(interactionServerCreateKey).(map[int64]any)
	if value != nil {
		value[0] = proc
	}
}
