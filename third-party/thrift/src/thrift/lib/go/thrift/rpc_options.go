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

// WithRPCOptions sets the RPCOptions in a client request go context
func WithRPCOptions(ctx context.Context, opts *RPCOptions) context.Context {
	return types.WithRPCOptions(ctx, opts)
}

// GetRPCOptions returns the RPCOptions in a go context, or nil if there is nothing
func GetRPCOptions(ctx context.Context) *RPCOptions {
	return types.GetRPCOptions(ctx)
}
