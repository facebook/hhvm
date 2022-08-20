/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

type interceptorProcessor struct {
	interceptor Interceptor
	Processor
}

// WrapInterceptor wraps an interceptor around the Processor p
// such as when running the method returned by that processor it will execute
// the interceptor instead. The interceptor is executed with
// context.Background() as its context.
func WrapInterceptor(interceptor Interceptor, p Processor) Processor {
	if interceptor == nil {
		return p
	}
	return &interceptorProcessor{
		interceptor: interceptor,
		Processor:   p,
	}
}

func (p *interceptorProcessor) GetProcessorFunction(name string) (ProcessorFunction, error) {
	pf, err := p.Processor.GetProcessorFunction(name)
	if err != nil {
		return nil, err
	}
	if pf == nil {
		return nil, nil
	}
	return &interceptorProcessorFunction{
		interceptor:       p.interceptor,
		methodName:        name,
		ProcessorFunction: pf,
	}, nil
}

type interceptorProcessorFunction struct {
	interceptor Interceptor
	methodName  string
	ProcessorFunction
}

func (pf *interceptorProcessorFunction) Run(args Struct) (WritableStruct, ApplicationException) {
	ctxPf := NewProcessorFunctionContextAdapter(pf.ProcessorFunction)
	return pf.interceptor(context.Background(), pf.methodName, ctxPf, args)
}

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

func (p *interceptorProcessorContext) GetProcessorFunctionContext(name string) (ProcessorFunctionContext, error) {
	pf, err := p.ProcessorContext.GetProcessorFunctionContext(name)
	if err != nil {
		return nil, err
	}
	if pf == nil {
		return nil, nil // see ProcessContext, this semantic means 'no such function'.
	}
	return &interceptorProcessorFunctionContext{
		interceptor:              p.interceptor,
		methodName:               name,
		ProcessorFunctionContext: pf,
	}, nil
}

type interceptorProcessorFunctionContext struct {
	interceptor Interceptor
	methodName  string
	ProcessorFunctionContext
}

func (pf *interceptorProcessorFunctionContext) RunContext(ctx context.Context, args Struct) (WritableStruct, ApplicationException) {
	return pf.interceptor(ctx, pf.methodName, pf.ProcessorFunctionContext, args)
}
