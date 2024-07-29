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
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"maps"
	"net"
	"time"

	rsocket "github.com/rsocket/rsocket-go"
	"github.com/rsocket/rsocket-go/core/transport"
	"github.com/rsocket/rsocket-go/payload"
)

type rocketClient struct {
	Encoder
	Decoder

	conn net.Conn

	// rsocket client state
	ctx    context.Context
	cancel func()
	client rsocket.Client

	resultChan chan rsocketResult

	timeout time.Duration

	protoID ProtocolID
	zstd    bool

	reqMetadata *requestRPCMetadata
	respHeaders map[string]string
	seqID       int32

	persistentHeaders map[string]string

	rbuf *MemoryBuffer
	wbuf *MemoryBuffer
}

// newRocketClient creates a RocketClient
func newRocketClient(conn net.Conn, protoID ProtocolID, timeout time.Duration, persistentHeaders map[string]string) (Protocol, error) {
	p := &rocketClient{
		conn:              conn,
		protoID:           protoID,
		persistentHeaders: persistentHeaders,
		rbuf:              NewMemoryBuffer(),
		wbuf:              NewMemoryBuffer(),
		timeout:           timeout,
		zstd:              false, // zstd adds a performance overhead, so we default to false
	}
	switch p.protoID {
	case ProtocolIDBinary:
		p.Decoder = newBinaryDecoder(p.rbuf)
		p.Encoder = newBinaryEncoder(p.wbuf)
	case ProtocolIDCompact:
		p.Decoder = newCompactDecoder(p.rbuf)
		p.Encoder = newCompactEncoder(p.wbuf)
	default:
		return nil, NewProtocolException(fmt.Errorf("Unknown protocol id: %#x", p.protoID))
	}
	return p, nil
}

type rsocketResult struct {
	val payload.Payload
	err error
}

func (p *rocketClient) WriteMessageBegin(name string, typeID MessageType, seqid int32) error {
	p.wbuf.Reset()
	p.seqID = seqid

	if p.reqMetadata == nil {
		p.reqMetadata = &requestRPCMetadata{}
	}
	p.reqMetadata.Name = name
	p.reqMetadata.TypeID = typeID
	p.reqMetadata.ProtoID = p.protoID
	p.reqMetadata.Zstd = p.zstd
	return nil
}

func (p *rocketClient) WriteMessageEnd() error {
	return nil
}

func (p *rocketClient) Flush() (err error) {
	if p.reqMetadata == nil {
		p.reqMetadata = &requestRPCMetadata{}
	}
	if p.reqMetadata.Other == nil {
		p.reqMetadata.Other = make(map[string]string)
	}
	for k, v := range p.persistentHeaders {
		p.reqMetadata.Other[k] = v
	}
	metadataBytes, err := serializeRequestRPCMetadata(p.reqMetadata)
	if err != nil {
		return err
	}
	// serializer in the protocol field was writing to the transport's memory buffer.
	dataBytes := p.wbuf.Bytes()
	if p.reqMetadata.Zstd {
		dataBytes, err = compressZstd(dataBytes)
		if err != nil {
			return err
		}
	}

	request := payload.New(dataBytes, metadataBytes)
	p.resultChan = make(chan rsocketResult, 1)
	if err := p.open(); err != nil {
		return err
	}
	// It is necessary to reset the deadline to 0.
	// The rsocket library only sets the deadline at connection start.
	// This means if you wait long enough, the connection will become useless.
	// Or something else is happening, but this is very necessary.
	p.conn.SetDeadline(time.Time{})
	mono := p.client.RequestResponse(request)
	if p.reqMetadata.TypeID != CALL {
		return nil
	}
	ctx := p.ctx
	go func() {
		val, err := mono.Block(ctx)
		val = payload.Clone(val)
		p.resultChan <- rsocketResult{val: val, err: err}
	}()
	return nil
}

// Open opens the internal transport (required for Transport)
func (p *rocketClient) open() error {
	if p.client != nil {
		return nil
	}
	setupPayload, err := newRequestSetupPayloadVersion8()
	if err != nil {
		return err
	}
	transporter := func(ctx context.Context) (*transport.Transport, error) {
		conn := transport.NewTCPClientTransport(p.conn)
		conn.SetLifetime(time.Millisecond * 3600000)
		return conn, nil
	}
	p.ctx, p.cancel = context.WithCancel(context.Background())
	clientBuilder := rsocket.Connect()
	// See T182939211. This copies the keep alives from Java Rocket.
	// KeepaliveLifetime = time.Duration(missedAcks = 1) * (ackTimeout = 3600000)
	clientBuilder = clientBuilder.KeepAlive(time.Millisecond*30000, time.Millisecond*3600000, 1)
	clientBuilder = clientBuilder.SetupPayload(setupPayload)
	clientBuilder = clientBuilder.OnClose(func(err error) {
		p.cancel()
	})
	clientStarter := clientBuilder.
		Acceptor(func(ctx context.Context, socket rsocket.RSocket) rsocket.RSocket {
			return rsocket.NewAbstractSocket(
				rsocket.MetadataPush(func(pay payload.Payload) {
					pay = payload.Clone(pay)
					// For documentation/reference see the CPP implementation
					// https://www.internalfb.com/code/fbsource/[ec968d3ea0ab]/fbcode/thrift/lib/cpp2/transport/rocket/client/RocketClient.cpp?lines=181
					metadataBytes, ok := pay.Metadata()
					if !ok {
						panic("no metadata in metadata push")
					}
					// Use ServerPushMetadata{} and do not use &ServerPushMetadata{} to ensure stack and avoid heap allocation.
					metadata := ServerPushMetadata{}
					if err := deserializeCompact(metadataBytes, &metadata); err != nil {
						panic(fmt.Errorf("unable to deserialize metadata push into ServerPushMetadata %w", err))
					}
					if metadata.SetupResponse != nil {
						// If zstdSupported is not set (or if false) client SHOULD not use ZSTD compression.
						if metadata.SetupResponse.ZstdSupported == nil {
							p.zstd = false
						} else {
							p.zstd = p.zstd && *metadata.SetupResponse.ZstdSupported
						}
						if metadata.SetupResponse.Version != nil {
							if *metadata.SetupResponse.Version != 8 {
								panic(fmt.Errorf("unsupported protocol version received in metadata push: %d", *metadata.SetupResponse.Version))
							}
						}
					} else if metadata.StreamHeadersPush != nil {
						panic("metadata push: StreamHeadersPush not implemented")
					} else if metadata.DrainCompletePush != nil {
						p.Close()
					}
				}),
			)
		})
	p.client, err = clientStarter.Transport(transporter).Start(p.ctx)
	return err
}

func (p *rocketClient) readPayload() (resp payload.Payload, err error) {
	ctx := p.ctx
	if p.timeout > 0 {
		var cancel context.CancelFunc
		ctx, cancel = context.WithTimeout(ctx, p.timeout)
		defer cancel()
	}
	select {
	case r := <-p.resultChan:
		return r.val, r.err
	case <-ctx.Done():
		return nil, ctx.Err()
	}
}

func (p *rocketClient) ReadMessageBegin() (string, MessageType, int32, error) {
	name := p.reqMetadata.Name
	resp, err := p.readPayload()
	if err != nil {
		return name, EXCEPTION, p.seqID, err
	}
	metadata := &ResponseRpcMetadata{}
	metadataBytes, ok := resp.Metadata()
	if ok {
		if err = deserializeCompact(metadataBytes, metadata); err != nil {
			return name, EXCEPTION, p.seqID, err
		}
		p.respHeaders = make(map[string]string)
		maps.Copy(p.respHeaders, metadata.OtherMetadata)
		if metadata.PayloadMetadata != nil && metadata.PayloadMetadata.ExceptionMetadata != nil {
			exception := newRocketException(metadata.PayloadMetadata.ExceptionMetadata)
			exceptionMetadata := metadata.PayloadMetadata.ExceptionMetadata
			if exceptionMetadata.Metadata != nil {
				if exceptionMetadata.Metadata.AppUnknownException != nil {
					return name, EXCEPTION, p.seqID, exception
				}
				// This is necessary for SR Proxy timeouts to work
				if exceptionMetadata.Metadata.DEPRECATEDProxyException != nil {
					return name, EXCEPTION, p.seqID, exception
				}
			}
		}
	}
	dataBytes := resp.Data()
	if metadata.Compression != nil && *metadata.Compression == CompressionAlgorithm_ZSTD {
		dataBytes, err = decompressZstd(dataBytes)
		if err != nil {
			return name, EXCEPTION, p.seqID, err
		}
	}
	p.rbuf.Buffer = bytes.NewBuffer(dataBytes)
	p.rbuf.size = len(dataBytes)
	return name, REPLY, p.seqID, err
}

type rocketException struct {
	Name          string
	What          string
	ExceptionType string
	Kind          string
	Blame         string
	Safety        string
}

func (e *rocketException) Error() string {
	data, err := json.Marshal(e)
	if err != nil {
		panic(err)
	}
	return string(data)
}

func newRocketException(exception *PayloadExceptionMetadataBase) error {
	err := &rocketException{
		Name:          "unknown",
		What:          "unknown",
		ExceptionType: "unknown",
		Kind:          "none",
		Blame:         "none",
		Safety:        "none",
	}
	if exception.NameUTF8 != nil {
		err.Name = *exception.NameUTF8
	}
	if exception.WhatUTF8 != nil {
		err.What = *exception.WhatUTF8
	}
	var class *ErrorClassification
	if exception.Metadata != nil {
		if exception.Metadata.DeclaredException != nil {
			err.ExceptionType = "DeclaredException"
			if exception.Metadata.DeclaredException.ErrorClassification != nil {
				class = exception.Metadata.DeclaredException.ErrorClassification
			}
		} else if exception.Metadata.AppUnknownException != nil {
			err.ExceptionType = "AppUnknownException"
			if exception.Metadata.AppUnknownException.ErrorClassification != nil {
				class = exception.Metadata.AppUnknownException.ErrorClassification
			}
		} else if exception.Metadata.AnyException != nil {
			err.ExceptionType = "AnyException"
		} else if exception.Metadata.DEPRECATEDProxyException != nil {
			err.ExceptionType = "DEPRECATEDProxyException"
		}
		if class != nil {
			if class.Kind != nil {
				err.Kind = class.Kind.String()
			}
			if class.Blame != nil {
				err.Blame = class.Blame.String()
			}
			if class.Safety != nil {
				err.Safety = class.Safety.String()
			}
		}
	}
	return NewTransportExceptionFromError(err)
}

func (p *rocketClient) ReadMessageEnd() error {
	p.reqMetadata = nil
	return nil
}

func (p *rocketClient) Skip(fieldType Type) (err error) {
	return SkipDefaultDepth(p, fieldType)
}

func (p *rocketClient) SetPersistentHeader(key, value string) {
	p.persistentHeaders[key] = value
}

func (p *rocketClient) GetPersistentHeader(key string) (string, bool) {
	v, ok := p.persistentHeaders[key]
	return v, ok
}

func (p *rocketClient) SetRequestHeader(key, value string) {
	if p.reqMetadata == nil {
		p.reqMetadata = &requestRPCMetadata{}
	}
	if p.reqMetadata.Other == nil {
		p.reqMetadata.Other = make(map[string]string)
	}
	p.reqMetadata.Other[key] = value
}

func (p *rocketClient) GetRequestHeaders() map[string]string {
	return p.reqMetadata.Other
}

func (p *rocketClient) GetResponseHeaders() map[string]string {
	return p.respHeaders
}

func (p *rocketClient) Close() error {
	if err := p.conn.Close(); err != nil {
		return err
	}
	if p.client != nil {
		if err := p.client.Close(); err != nil {
			return err
		}
		p.client = nil
	}
	if p.cancel != nil {
		p.cancel()
	}
	return nil
}
