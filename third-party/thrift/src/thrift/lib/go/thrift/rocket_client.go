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
	"errors"
	"fmt"
	"maps"
	"net"
	"runtime"
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/format"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
)

type rocketClient struct {
	types.Encoder
	types.Decoder

	// rsocket client state
	client RSocketClient
	// Handle containing the cleanup (.Close call) for the 'client' (RSocketClient) above,
	// for when the enclosing 'rocketClient' object goes out of scope, and in case the user
	// forgets to explicitly close the client.
	// This cleanup is VERY IMPORTANT - not cleaning up can lead to Goroutine and FD leaks!
	clientCleanup runtime.Cleanup

	ioTimeout time.Duration

	protoID types.ProtocolID

	persistentHeaders map[string]string

	// NOTE: all variables below are used per-request.
	messageName string
	writeType   types.MessageType
	seqID       int32
	resultData  []byte
	resultErr   error
	reqHeaders  map[string]string
	respHeaders map[string]string
	rbuf        *bytes.Buffer
	wbuf        *bytes.Buffer
}

var _ Protocol = (*rocketClient)(nil)
var _ RequestChannel = (*rocketClient)(nil)

func newRocketClient(
	conn net.Conn,
	protoID types.ProtocolID,
	ioTimeout time.Duration,
	persistentHeaders map[string]string,
) (Protocol, error) {
	var rpcProtocolID rpcmetadata.ProtocolId
	switch protoID {
	case types.ProtocolIDBinary:
		rpcProtocolID = rpcmetadata.ProtocolId_BINARY
	case types.ProtocolIDCompact:
		rpcProtocolID = rpcmetadata.ProtocolId_COMPACT
	default:
		return nil, fmt.Errorf("unsupported ProtocolID: %d", protoID)
	}
	return newRocketClientFromRsocket(newRSocketClient(conn, rpcProtocolID), protoID, ioTimeout, persistentHeaders)
}

func newRocketClientFromRsocket(
	client RSocketClient,
	protoID types.ProtocolID,
	ioTimeout time.Duration,
	persistentHeaders map[string]string,
) (Protocol, error) {
	p := &rocketClient{
		client:            client,
		protoID:           protoID,
		persistentHeaders: persistentHeaders,
		rbuf:              new(bytes.Buffer),
		wbuf:              new(bytes.Buffer),
		ioTimeout:         ioTimeout,
		reqHeaders:        make(map[string]string),
	}
	switch p.protoID {
	case types.ProtocolIDBinary:
		p.Decoder = format.NewBinaryDecoder(p.rbuf)
		p.Encoder = format.NewBinaryEncoder(p.wbuf)
	case types.ProtocolIDCompact:
		p.Decoder = format.NewCompactDecoder(p.rbuf)
		p.Encoder = format.NewCompactEncoder(p.wbuf)
	default:
		return nil, types.NewProtocolException(fmt.Errorf("Unknown protocol id: %d", p.protoID))
	}
	p.clientCleanup = runtime.AddCleanup(p,
		func(underlyingClient RSocketClient) {
			underlyingClient.Close()
		}, client)
	return p, nil
}

func (p *rocketClient) WriteMessageBegin(name string, typeID types.MessageType, seqid int32) error {
	p.wbuf.Reset()
	p.seqID = seqid
	p.writeType = typeID
	p.messageName = name
	return nil
}

func (p *rocketClient) WriteMessageEnd() error {
	return nil
}

func (p *rocketClient) Flush() error {
	dataBytes := p.wbuf.Bytes()

	ctx := context.Background()
	if p.ioTimeout > 0 {
		var cancel context.CancelFunc
		ctx, cancel = context.WithTimeout(ctx, p.ioTimeout)
		defer cancel()
	}

	if err := p.client.SendSetup(ctx); err != nil {
		return err
	}
	headers := unionMaps(p.reqHeaders, p.persistentHeaders)
	if p.writeType == types.ONEWAY {
		return p.client.FireAndForget(p.messageName, headers, dataBytes)
	}
	if p.writeType != types.CALL {
		return nil
	}
	p.respHeaders, p.resultData, p.resultErr = p.client.RequestResponse(ctx, p.messageName, headers, dataBytes)
	clear(p.reqHeaders)
	return nil
}

func (p *rocketClient) SendRequestNoResponse(ctx context.Context, messageName string, request WritableStruct) error {
	rpcOpts := GetRPCOptions(ctx)
	dataBytes, err := encodeRequest(p.protoID, request)
	if err != nil {
		return err
	}

	if p.ioTimeout > 0 {
		var cancel context.CancelFunc
		ctx, cancel = context.WithTimeout(ctx, p.ioTimeout)
		defer cancel()
	}

	err = p.client.SendSetup(ctx)
	if err != nil {
		return err
	}

	var writeHeaders map[string]string
	if rpcOpts != nil {
		writeHeaders = rpcOpts.GetWriteHeaders()
	}
	headers := unionMaps(writeHeaders, p.persistentHeaders)
	return p.client.FireAndForget(messageName, headers, dataBytes)
}

func (p *rocketClient) SendRequestResponse(ctx context.Context, messageName string, request WritableStruct, response ReadableStruct) error {
	rpcOpts := GetRPCOptions(ctx)
	dataBytes, err := encodeRequest(p.protoID, request)
	if err != nil {
		return err
	}

	if p.ioTimeout > 0 {
		var cancel context.CancelFunc
		ctx, cancel = context.WithTimeout(ctx, p.ioTimeout)
		defer cancel()
	}

	err = p.client.SendSetup(ctx)
	if err != nil {
		return err
	}

	var writeHeaders map[string]string
	if rpcOpts != nil {
		writeHeaders = rpcOpts.GetWriteHeaders()
	}
	headers := unionMaps(writeHeaders, p.persistentHeaders)
	respHeaders, resultData, resultErr := p.client.RequestResponse(ctx, messageName, headers, dataBytes)
	if resultErr != nil {
		return resultErr
	}

	if rpcOpts != nil {
		rpcOpts.SetReadHeaders(respHeaders)
	}
	err = decodeResponse(p.protoID, resultData, response)
	if err != nil {
		return err
	}
	return nil
}

func (p *rocketClient) SendRequestStream(
	ctx context.Context,
	messageName string,
	request WritableStruct,
	response ReadableStruct,
	onStreamNextFn func(Decoder) error,
	onStreamErrorFn func(error),
	onStreamCompleteFn func(),
) error {
	if ctx.Done() == nil {
		// We require that the context is cancellable, to prevent goroutine leaks.
		return errors.New("context does not support cancellation")
	}
	rpcOpts := GetRPCOptions(ctx)

	dataBytes, err := encodeRequest(p.protoID, request)
	if err != nil {
		return err
	}

	var cancelFunc context.CancelFunc
	if p.ioTimeout > 0 {
		ctx, cancelFunc = context.WithTimeout(ctx, p.ioTimeout)
		defer func() {
			if cancelFunc != nil {
				cancelFunc()
			}
		}()
	}

	err = p.client.SendSetup(ctx)
	if err != nil {
		return err
	}
	onStreamNextWrapperFn := func(data []byte) error {
		reader := bytes.NewBuffer(data)
		var decoder types.Decoder
		switch p.protoID {
		case types.ProtocolIDBinary:
			decoder = format.NewBinaryDecoder(reader)
		case types.ProtocolIDCompact:
			decoder = format.NewCompactDecoder(reader)
		default:
			return types.NewProtocolException(fmt.Errorf("Unknown protocol id: %d", p.protoID))
		}
		return onStreamNextFn(decoder)
	}

	var writeHeaders map[string]string
	if rpcOpts != nil {
		writeHeaders = rpcOpts.GetWriteHeaders()
	}
	headers := unionMaps(writeHeaders, p.persistentHeaders)
	respHeaders, resultData, resultErr := p.client.RequestStream(ctx, messageName, headers, dataBytes, onStreamNextWrapperFn, onStreamErrorFn, onStreamCompleteFn)
	if resultErr != nil {
		return resultErr
	}

	if rpcOpts != nil {
		rpcOpts.SetReadHeaders(respHeaders)
	}
	err = decodeResponse(p.protoID, resultData, response)
	if err != nil {
		return err
	}
	// Do not cancel the context to allow streaming to continue
	cancelFunc = nil
	return nil
}

func encodeRequest(protoID types.ProtocolID, request WritableStruct) ([]byte, error) {
	switch protoID {
	case types.ProtocolIDBinary:
		return EncodeBinary(request)
	case types.ProtocolIDCompact:
		return EncodeCompact(request)
	default:
		return nil, types.NewProtocolException(fmt.Errorf("Unknown protocol id: %d", protoID))
	}
}

func decodeResponse(protoID types.ProtocolID, data []byte, response ReadableStruct) error {
	switch protoID {
	case types.ProtocolIDBinary:
		return DecodeBinary(data, response)
	case types.ProtocolIDCompact:
		return DecodeCompact(data, response)
	default:
		return types.NewProtocolException(fmt.Errorf("Unknown protocol id: %d", protoID))
	}
}

func unionMaps(args ...map[string]string) map[string]string {
	// Creates a brand new unified map and copies contents of 'args' into it.
	unifiedMap := make(map[string]string)
	for _, arg := range args {
		maps.Copy(unifiedMap, arg)
	}
	return unifiedMap
}

func (p *rocketClient) ReadMessageBegin() (string, types.MessageType, int32, error) {
	name := p.messageName
	if p.resultErr != nil {
		return name, types.EXCEPTION, p.seqID, p.resultErr
	}

	// Clear the buffer, but keep the underlying storage.
	p.rbuf.Reset()
	// Write the result data into the buffer.
	_, err := p.rbuf.Write(p.resultData)
	if err != nil {
		return name, types.EXCEPTION, p.seqID, err
	}
	return name, types.REPLY, p.seqID, nil
}

func (p *rocketClient) ReadMessageEnd() error {
	return nil
}

func (p *rocketClient) Skip(fieldType types.Type) error {
	return types.SkipDefaultDepth(p, fieldType)
}

func (p *rocketClient) setRequestHeader(key, value string) {
	p.reqHeaders[key] = value
}

func (p *rocketClient) getResponseHeaders() map[string]string {
	return p.respHeaders
}

func (p *rocketClient) Close() error {
	// no need for the cleanup anymore (idempotent method)
	p.clientCleanup.Stop()
	return p.client.Close()
}
