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

// Interceptor is a function that runs before the actual method. It is passed
// the connection context, the method name and the args for that method.
// The interceptor is responsible for calling pfunc.RunContext() and it can
// return a result or an exception which are then sent back to the caller.
// The interceptor is expected to be concurrency safe.
type Interceptor func(
	ctx context.Context,
	methodName string,
	pfunc types.ProcessorFunction,
	args types.ReadableStruct,
) (types.WritableStruct, types.ApplicationExceptionIf)

type interceptorProcessor struct {
	// Embeds original Processor
	Processor

	interceptor Interceptor
}

// WrapInterceptor wraps an interceptor around the Processor p
// such as when running the method returned by that processor it will execute
// the interceptor instead.
func WrapInterceptor(interceptor Interceptor, p Processor) Processor {
	if interceptor == nil {
		return p
	}
	return &interceptorProcessor{
		Processor:   p,
		interceptor: interceptor,
	}
}

func (p *interceptorProcessor) ProcessorFunctionMap() map[string]types.ProcessorFunction {
	m := p.Processor.ProcessorFunctionMap()
	mi := make(map[string]types.ProcessorFunction)
	for name, pf := range m {
		mi[name] = &interceptorProcessorFunction{
			ProcessorFunction: pf,
			interceptor:       p.interceptor,
			methodName:        name,
		}
	}
	return mi
}

type interceptorProcessorFunction struct {
	// Embeds original ProcessorFunction
	types.ProcessorFunction

	interceptor Interceptor
	methodName  string
}

func (pf *interceptorProcessorFunction) RunContext(ctx context.Context, args types.ReadableStruct) (types.WritableStruct, types.ApplicationExceptionIf) {
	return pf.interceptor(ctx, pf.methodName, pf.ProcessorFunction, args)
}
