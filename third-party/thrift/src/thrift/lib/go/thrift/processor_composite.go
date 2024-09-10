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

// CompositeProcessor is a serial Processor can report what functions it has
// Currently all generated Go processors satisfy this interface.
type CompositeProcessor interface {
	Processor
	Include(processor map[string]ProcessorFunction)
	ProcessorFunctionMap() map[string]ProcessorFunction
}

// compositeProcessor allows different ComposableProcessor to sit under one
// server as long as their functions carry distinct names
type compositeProcessor struct {
	serviceProcessorMap map[string]ProcessorFunction
}

// NewCompositeProcessor creates a new CompositeProcessor
func NewCompositeProcessor() CompositeProcessor {
	return &compositeProcessor{
		serviceProcessorMap: make(map[string]ProcessorFunction),
	}
}

// Include registers the given processor's functions in the CompositeProcessor
// This silently overrides collided function names (last processor to
// include wins).
// A full solution (inclusion respecting namespaces) will require changes
// to the thrift compiler
func (p *compositeProcessor) Include(processorMap map[string]ProcessorFunction) {
	for name, tfunc := range processorMap {
		p.serviceProcessorMap[name] = tfunc
	}
}

// GetProcessorFunction multiplexes redirects to the appropriate Processor
func (p *compositeProcessor) GetProcessorFunction(name string) ProcessorFunction {
	tfunc, _ := p.serviceProcessorMap[name]
	return tfunc
}

// ProcessorMap returns the map that maps method names to Processors
func (p *compositeProcessor) ProcessorFunctionMap() map[string]ProcessorFunction {
	return p.serviceProcessorMap
}
