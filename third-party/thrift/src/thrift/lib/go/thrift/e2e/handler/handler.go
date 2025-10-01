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

package handler

import (
	"context"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift"
	"thrift/lib/go/thrift/e2e/service"
)

type E2EHandler struct{}

// Compile time interface enforcer
var _ service.E2E = (*E2EHandler)(nil)

func (h *E2EHandler) EchoHeaders(ctx context.Context) (map[string]string, error) {
	return thrift.GetRequestHeadersFromContext(ctx), nil
}
