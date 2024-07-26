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
	"testing"
)

type exampleProcessor struct {
	ProcessorContext
	hit *bool
}

func (ep *exampleProcessor) GetProcessorFunctionContext(name string) ProcessorFunctionContext {
	*ep.hit = true
	return nil // happens in "no such method" case
}

func TestInterceptorWrapperNilFunctionContext(t *testing.T) {
	var hit bool
	proc := &exampleProcessor{nil, &hit}

	derivedProc := WrapInterceptorContext(emptyInterceptor, proc)
	pFunc := derivedProc.GetProcessorFunctionContext("blah")
	if hit != true {
		t.Fatalf("interceptor should have called underlying processor function handler.")
	}
	if pFunc != nil {
		t.Fatalf("derived interceptor context should return underlying nil processor function context.")
	}
}

func emptyInterceptor(ctx context.Context, methodName string, pfunc ProcessorFunctionContext, args Struct) (WritableStruct, ApplicationException) {
	return pfunc.RunContext(ctx, args)
}
