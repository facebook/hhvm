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

// ProcessorFactory is the default processor factory which returns
// a singleton instance.
type ProcessorFactory interface {
	GetProcessor(trans Transport) Processor
}

type processorFactory struct {
	processor Processor
}

// NewProcessorFactory returns a ProcessorFactory.
func NewProcessorFactory(p Processor) ProcessorFactory {
	return &processorFactory{processor: p}
}

func (p *processorFactory) GetProcessor(trans Transport) Processor {
	return p.processor
}

/**
 * The default processor factory just returns a singleton
 * instance.
 */
type ProcessorFunctionFactory interface {
	GetProcessorFunction(trans Transport) ProcessorFunction
}

type processorFunctionFactory struct {
	processor ProcessorFunction
}

func NewProcessorFunctionFactory(p ProcessorFunction) ProcessorFunctionFactory {
	return &processorFunctionFactory{processor: p}
}

func (p *processorFunctionFactory) GetProcessorFunction(trans Transport) ProcessorFunction {
	return p.processor
}

// ProcessorFactoryContext is a ProcessorFactory that supports contexts.
type ProcessorFactoryContext interface {
	GetProcessorContext(trans Transport) ProcessorContext
}

type processorFactoryContext struct {
	processorContext ProcessorContext
}

// NewProcessorFactoryContext returns a ProcessorFactoryContext.
func NewProcessorFactoryContext(p ProcessorContext) ProcessorFactoryContext {
	return &processorFactoryContext{processorContext: p}
}

func (p *processorFactoryContext) GetProcessorContext(trans Transport) ProcessorContext {
	return p.processorContext
}

// NewProcessorFactoryContextAdapter creates a ProcessorFactoryContext from a regular ProcessorFactory.
func NewProcessorFactoryContextAdapter(p ProcessorFactory) ProcessorFactoryContext {
	return &ctxProcessorFactoryAdapter{p}
}

type ctxProcessorFactoryAdapter struct {
	ProcessorFactory
}

func (p ctxProcessorFactoryAdapter) GetProcessorContext(trans Transport) ProcessorContext {
	return NewProcessorContextAdapter(p.ProcessorFactory.GetProcessor(trans))
}
