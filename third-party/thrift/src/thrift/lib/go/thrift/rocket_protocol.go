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
	"fmt"

	"github.com/rsocket/rsocket-go/payload"
	"github.com/rsocket/rsocket-go/rx"
)

type rocketProtocol struct {
	Protocol
	trans        *rocketTransport
	responseChan chan payload.Payload
	errChan      chan error

	protoID      ProtocolID
	reqMetadata  *requestRPCMetadata
	respMetadata *ResponseRpcMetadata
	seqID        int32

	persistentHeaders map[string]string
}

type rocketProtocolFactory struct{}

// NewRocketProtocolFactory makes it possible to create a RocketProtocol using a RocketTransport
func NewRocketProtocolFactory() ProtocolFactory {
	return &rocketProtocolFactory{}
}

func (p *rocketProtocolFactory) GetProtocol(trans Transport) Protocol {
	return NewRocketProtocol(trans)
}

// NewRocketProtocol creates a RocketProtocol, given a RocketTransport
func NewRocketProtocol(trans Transport) Protocol {
	p := &rocketProtocol{
		protoID:           ProtocolIDCompact,
		persistentHeaders: make(map[string]string),
	}
	switch t := trans.(type) {
	case *rocketTransport:
		p.trans = t
	default:
		panic(NewTransportException(
			NOT_IMPLEMENTED,
			fmt.Sprintf("Rocket Transport only supports Socket and does not support: %T", trans)))
	}

	if err := p.resetProtocol(); err != nil {
		panic(err)
	}
	return p
}

func (p *rocketProtocol) resetProtocol() error {
	p.trans.resetBuffers()
	if p.Protocol != nil && p.protoID == p.trans.ProtocolID() {
		return nil
	}

	p.protoID = p.trans.ProtocolID()
	switch p.protoID {
	case ProtocolIDBinary:
		// These defaults match cpp implementation
		p.Protocol = NewBinaryProtocol(p.trans, false, true)
	case ProtocolIDCompact:
		p.Protocol = NewCompactProtocol(p.trans)
	default:
		return NewProtocolException(fmt.Errorf("Unknown protocol id: %#x", p.protoID))
	}
	return nil
}

func (p *rocketProtocol) WriteMessageBegin(name string, typeID MessageType, seqid int32) error {
	if err := p.resetProtocol(); err != nil {
		return err
	}
	p.seqID = seqid

	if p.reqMetadata == nil {
		p.reqMetadata = &requestRPCMetadata{}
	}
	p.reqMetadata.Name = name
	p.reqMetadata.TypeID = typeID
	p.reqMetadata.ProtoID = p.protoID
	p.reqMetadata.Zstd = true
	return nil
}

func (p *rocketProtocol) WriteMessageEnd() error {
	return nil
}

func (p *rocketProtocol) Flush() (err error) {
	for k, v := range p.persistentHeaders {
		p.reqMetadata.Other[k] = v
	}
	metadataBytes, err := serializeRequestRPCMetadata(p.reqMetadata)
	if err != nil {
		return err
	}
	// serializer in the protocol field was writing to the transport's memory buffer.
	dataBytes := p.trans.wbuf.Bytes()
	if p.reqMetadata.Zstd {
		dataBytes, err = compressZstd(dataBytes)
		if err != nil {
			return err
		}
	}

	request := payload.New(dataBytes, metadataBytes)
	p.responseChan = make(chan payload.Payload, 1)
	p.errChan = make(chan error, 1)
	if !p.trans.IsOpen() {
		if err := p.trans.Open(); err != nil {
			return err
		}
	}
	p.trans.client.RequestResponse(request).Subscribe(p.trans.ctx, rx.OnNext(
		func(response payload.Payload) error {
			metadata, _ := response.Metadata()
			data := response.Data()
			newPayload := payload.New(data, metadata)
			p.responseChan <- newPayload
			return nil
		}),
		rx.OnError(func(e error) {
			p.errChan <- e
		}),
		rx.OnComplete(func() {
			close(p.responseChan)
			close(p.errChan)
		}))

	return NewProtocolException(p.trans.Flush())
}

func (p *rocketProtocol) readPayload() (resp payload.Payload, err error) {
	respDone := false
	errDone := false
	for !respDone || !errDone {
		select {
		case v, ok := <-p.responseChan:
			if ok {
				resp = v
			} else {
				respDone = true
			}
		case v, ok := <-p.errChan:
			if ok {
				err = v
			} else {
				errDone = true
			}
		}
	}
	return resp, err
}

func (p *rocketProtocol) ReadMessageBegin() (string, MessageType, int32, error) {
	resp, err := p.readPayload()
	if err != nil {
		return "", EXCEPTION, 0, err
	}
	metadataBytes, ok := resp.Metadata()
	if ok {
		metadata := &ResponseRpcMetadata{}
		compactDeserializer := NewCompactDeserializer()
		if err = compactDeserializer.Read(metadata, metadataBytes); err != nil {
			return "", EXCEPTION, 0, err
		}
		p.respMetadata = metadata
	}
	name := p.reqMetadata.Name
	dataBytes := resp.Data()
	if p.respMetadata.Compression != nil && *p.respMetadata.Compression == CompressionAlgorithm_ZSTD {
		dataBytes, err = decompressZstd(dataBytes)
		if err != nil {
			return name, EXCEPTION, 0, err
		}
	}
	p.trans.setReadBuf(dataBytes)
	return name, REPLY, p.seqID, err
}

func (p *rocketProtocol) ReadMessageEnd() error {
	p.reqMetadata = nil
	return nil
}

func (p *rocketProtocol) Skip(fieldType Type) (err error) {
	return SkipDefaultDepth(p, fieldType)
}

// Deprecated: Transport is a deprecated method
func (p *rocketProtocol) Transport() Transport {
	return nil
}

func (p *rocketProtocol) SetPersistentHeader(key, value string) {
	p.persistentHeaders[key] = value
}

func (p *rocketProtocol) GetPersistentHeader(key string) (string, bool) {
	v, ok := p.persistentHeaders[key]
	return v, ok
}

func (p *rocketProtocol) GetPersistentHeaders() map[string]string {
	return p.persistentHeaders
}

func (p *rocketProtocol) ClearPersistentHeaders() {
	p.persistentHeaders = make(map[string]string)
}

func (p *rocketProtocol) SetRequestHeader(key, value string) {
	if p.reqMetadata == nil {
		p.reqMetadata = &requestRPCMetadata{}
	}
	if p.reqMetadata.Other == nil {
		p.reqMetadata.Other = make(map[string]string)
	}
	p.reqMetadata.Other[key] = value
}

func (p *rocketProtocol) GetRequestHeader(key string) (value string, ok bool) {
	v, ok := p.GetRequestHeaders()[key]
	return v, ok
}

func (p *rocketProtocol) GetRequestHeaders() map[string]string {
	return p.reqMetadata.Other
}

func (p *rocketProtocol) GetResponseHeader(key string) (string, bool) {
	if p.respMetadata == nil {
		return "", false
	}
	value, ok := p.respMetadata.OtherMetadata[key]
	return value, ok
}

func (p *rocketProtocol) GetResponseHeaders() map[string]string {
	if p.respMetadata == nil {
		return nil
	}
	return p.respMetadata.OtherMetadata
}

func (p *rocketProtocol) Close() error {
	return p.trans.Close()
}
