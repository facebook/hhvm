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
	"fmt"
	"strings"
)

/*
MultiplexedProtocol is a protocol-independent concrete decorator
that allows a Thrift client to communicate with a multiplexing Thrift server,
by prepending the service name to the function name during function calls.

NOTE: THIS IS NOT USED BY SERVERS.  On the server, use MultiplexedProcessor to handle request
from a multiplexing client.

This example uses a single socket transport to invoke two services:

socket := thrift.NewSocket(thrift.SocketAddr(addr), thrif.SocketTimeout(TIMEOUT))
transport := thrift.NewFramedTransport(socket)
protocol := thrift.NewBinaryProtocolTransport(transport)

mp := thrift.NewMultiplexedProtocol(protocol, "Calculator")
service := Calculator.NewCalculatorClient(mp)

mp2 := thrift.NewMultiplexedProtocol(protocol, "WeatherReport")
service2 := WeatherReport.NewWeatherReportClient(mp2)

err := transport.Open()
if err != nil {
	t.Fatal("Unable to open client socket", err)
}

fmt.Println(service.Add(2,2))
fmt.Println(service2.GetTemperature())
*/

type MultiplexedProtocol struct {
	Protocol
	serviceName string
}

const MULTIPLEXED_SEPARATOR = ":"

func NewMultiplexedProtocol(protocol Protocol, serviceName string) *MultiplexedProtocol {
	return &MultiplexedProtocol{
		Protocol:    protocol,
		serviceName: serviceName,
	}
}

func (t *MultiplexedProtocol) WriteMessageBegin(name string, typeId MessageType, seqid int32) error {
	if typeId == CALL || typeId == ONEWAY {
		return t.Protocol.WriteMessageBegin(t.serviceName+MULTIPLEXED_SEPARATOR+name, typeId, seqid)
	} else {
		return t.Protocol.WriteMessageBegin(name, typeId, seqid)
	}
}

/*
MultiplexedProcessor is a Processor allowing
a single Server to provide multiple services.

To do so, you instantiate the processor and then register additional
processors with it, as shown in the following example:

var processor = thrift.NewMultiplexedProcessor()

firsprocessor :=
processor.RegisterProcessor("FirstService", firsprocessor)

processor.registerProcessor(
  "Calculator",
  Calculator.NewCalculatorProcessor(&CalculatorHandler{}),
)

processor.registerProcessor(
  "WeatherReport",
  WeatherReport.NewWeatherReporprocessor(&WeatherReportHandler{}),
)

serverTransport, err := thrift.NewServerSocketTimeout(addr, TIMEOUT)
if err != nil {
  t.Fatal("Unable to create server socket", err)
}
server := thrift.NewSimpleServer(processor, serverTransport)
server.Serve();
*/

type MultiplexedProcessor struct {
	serviceProcessorMap map[string]Processor
	Defaulprocessor     Processor
}

func NewMultiplexedProcessor() *MultiplexedProcessor {
	return &MultiplexedProcessor{
		serviceProcessorMap: make(map[string]Processor),
	}
}

func (t *MultiplexedProcessor) RegisterDefault(processor Processor) {
	t.Defaulprocessor = processor
}

func (t *MultiplexedProcessor) RegisterProcessor(name string, processor Processor) {
	if t.serviceProcessorMap == nil {
		t.serviceProcessorMap = make(map[string]Processor)
	}
	t.serviceProcessorMap[name] = processor
}

// GetProcessorFunction implements the thrift.Processor interface.  It parses the
// thrift function name to figure out which processor to route the request to and
// returns descriptive error messages to help clients diagnose errors.
func (t *MultiplexedProcessor) GetProcessorFunction(name string) (ProcessorFunction, error) {
	//extract the service name
	v := strings.SplitN(name, MULTIPLEXED_SEPARATOR, 2)
	if len(v) != 2 {
		if t.Defaulprocessor != nil {
			return t.Defaulprocessor.GetProcessorFunction(name)
		}
		return nil, fmt.Errorf("Service name not found in message name: %s.  Did you forget to use a MultiplexProtocol in your client?", name)
	}
	actualProcessor, ok := t.serviceProcessorMap[v[0]]
	if !ok {
		return nil, fmt.Errorf("Service name not found: %s.  Did you forget to call registerProcessor()?", v[0])
	}
	return actualProcessor.GetProcessorFunction(v[1])
}

//Protocol that use stored message for ReadMessageBegin
type storedMessageProtocol struct {
	Protocol
	name   string
	typeId MessageType
	seqid  int32
}

func NewStoredMessageProtocol(protocol Protocol, name string, typeId MessageType, seqid int32) *storedMessageProtocol {
	return &storedMessageProtocol{protocol, name, typeId, seqid}
}

func (s *storedMessageProtocol) ReadMessageBegin() (name string, typeId MessageType, seqid int32, err error) {
	return s.name, s.typeId, s.seqid, nil
}
