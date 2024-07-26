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
)

// Interceptor is a function that runs before the actual method. It is passed
// the connection context, the method name and the args for that method.
// The interceptor is responsible for calling pfunc.RunContext() and it can
// return a result or an exception which are then sent back to the caller.
// The interceptor is expected to be concurrency safe.
type Interceptor func(ctx context.Context, methodName string, pfunc ProcessorFunctionContext, args Struct) (WritableStruct, ApplicationException)

type interceptorProcessorContext struct {
	interceptor Interceptor
	ProcessorContext
}

// WrapInterceptorContext wraps an interceptor around the ProcessorContext p
// such as when running the method returned by that processor it will execute
// the interceptor instead.
func WrapInterceptorContext(interceptor Interceptor, p ProcessorContext) ProcessorContext {
	if interceptor == nil {
		return p
	}
	return &interceptorProcessorContext{
		interceptor:      interceptor,
		ProcessorContext: p,
	}
}

func (p *interceptorProcessorContext) GetProcessorFunctionContext(name string) ProcessorFunctionContext {
	pf := p.ProcessorContext.GetProcessorFunctionContext(name)
	if pf == nil {
		return nil // see ProcessContext, this semantic means 'no such function'.
	}
	return &interceptorProcessorFunctionContext{
		interceptor:              p.interceptor,
		methodName:               name,
		ProcessorFunctionContext: pf,
	}
}

type interceptorProcessorFunctionContext struct {
	interceptor Interceptor
	methodName  string
	ProcessorFunctionContext
}

func (pf *interceptorProcessorFunctionContext) RunContext(ctx context.Context, args Struct) (WritableStruct, ApplicationException) {
	return pf.interceptor(ctx, pf.methodName, pf.ProcessorFunctionContext, args)
}
