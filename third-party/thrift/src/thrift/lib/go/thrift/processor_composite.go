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
	"maps"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/metadata"
)

// CompositeProcessor is a serial Processor can report what functions it has
// Currently all generated Go processors satisfy this interface.
type CompositeProcessor interface {
	Processor
	Include(processor Processor)
}

// compositeProcessor allows different ComposableProcessor to sit under one
// server as long as their functions carry distinct names
type compositeProcessor struct {
	processorFunctionMap  map[string]types.ProcessorFunction
	functionServiceMap    map[string]string
	metadata              *metadata.ThriftMetadata
	interactionProcessors []types.Processor
}

// NewCompositeProcessor creates a new CompositeProcessor
func NewCompositeProcessor() CompositeProcessor {
	return &compositeProcessor{
		processorFunctionMap:  make(map[string]types.ProcessorFunction),
		functionServiceMap:    make(map[string]string),
		metadata:              metadata.NewThriftMetadata(),
		interactionProcessors: []types.Processor{},
	}
}

// Include registers the given processor's functions in the CompositeProcessor
// This silently overrides collided function names (last processor to
// include wins).
// A full solution (inclusion respecting namespaces) will require changes
// to the thrift compiler
func (p *compositeProcessor) Include(processor Processor) {
	maps.Copy(p.processorFunctionMap, processor.ProcessorFunctionMap())
	maps.Copy(p.functionServiceMap, processor.FunctionServiceMap())

	metadata := processor.GetThriftMetadata()
	maps.Copy(p.metadata.Enums, metadata.GetEnums())
	maps.Copy(p.metadata.Structs, metadata.GetStructs())
	maps.Copy(p.metadata.Exceptions, metadata.GetExceptions())
	maps.Copy(p.metadata.Services, metadata.GetServices())

	p.interactionProcessors = append(p.interactionProcessors, processor.GetInteractionProcessors()...)
}

// GetProcessorFunction multiplexes redirects to the appropriate Processor
func (p *compositeProcessor) GetProcessorFunction(name string) types.ProcessorFunction {
	return p.processorFunctionMap[name]
}

// ProcessorMap returns the map that maps method names to Processors
func (p *compositeProcessor) ProcessorFunctionMap() map[string]types.ProcessorFunction {
	return p.processorFunctionMap
}

// FunctionServiceMap returns the map that maps method names to service names
func (p *compositeProcessor) FunctionServiceMap() map[string]string {
	return p.functionServiceMap
}

func (p *compositeProcessor) GetThriftMetadata() *metadata.ThriftMetadata {
	return p.metadata
}

// GetInteractionProcessors returns the collected interaction processors
// from all included processors that provide interactions.
func (p *compositeProcessor) GetInteractionProcessors() []types.Processor {
	return p.interactionProcessors
}
