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
	"bufio"
	"bytes"
	"compress/zlib"
	"encoding/binary"
	"fmt"
	"io/ioutil"
)

const (
	DefaulprotoID     = ProtocolIDCompact
	DefaultClientType = HeaderClientType
)

type tHeaderTransportFactory struct {
	factory TransportFactory
}

func NewHeaderTransportFactory(factory TransportFactory) TransportFactory {
	return &tHeaderTransportFactory{factory: factory}
}

func (p *tHeaderTransportFactory) GetTransport(base Transport) Transport {
	return NewHeaderTransport(p.factory.GetTransport(base))
}

type HeaderTransport struct {
	transport Transport

	// Used on read
	rbuf       *bufio.Reader
	framebuf   byteReader
	readHeader *tHeader
	// remaining bytes in the current frame. If 0, read in a new frame.
	frameSize uint64

	// Used on write
	wbuf                       *bytes.Buffer
	identity                   string
	writeInfoHeaders           map[string]string
	persistentWriteInfoHeaders map[string]string

	// Negotiated
	protoID            ProtocolID
	readSeqID          uint32 // read (and written, if not set explicitly)
	writeSeqID         uint32 // written, if set by user of transport
	seqIDExplicitlySet bool
	flags              uint16
	clientType         ClientType
	writeTransforms    []TransformID
	firstRequest       bool
}

// NewHeaderTransport creates a new transport with defaults.
func NewHeaderTransport(transport Transport) *HeaderTransport {
	return &HeaderTransport{
		transport: transport,
		rbuf:      bufio.NewReader(transport),
		framebuf:  newLimitedByteReader(bytes.NewReader(nil), 0),
		frameSize: 0,

		wbuf:                       bytes.NewBuffer(nil),
		writeInfoHeaders:           map[string]string{},
		persistentWriteInfoHeaders: map[string]string{},

		protoID:         DefaulprotoID,
		flags:           0,
		clientType:      DefaultClientType,
		writeTransforms: []TransformID{},
		firstRequest:    true,
	}
}

func (t *HeaderTransport) SetSeqID(seq uint32) {
	t.seqIDExplicitlySet = true
	t.writeSeqID = seq
}

func (t *HeaderTransport) SeqID() uint32 {
	return t.readSeqID
}

func (t *HeaderTransport) Identity() string {
	return t.identity
}

func (t *HeaderTransport) SetIdentity(identity string) {
	t.identity = identity
}

func (t *HeaderTransport) peerIdentity() string {
	v, ok := t.ReadHeader(IdentityHeader)
	vers, versok := t.ReadHeader(IDVersionHeader)
	if ok && versok && vers == IDVersion {
		return v
	}
	return ""
}

func (t *HeaderTransport) SetPersistentHeader(key, value string) {
	t.persistentWriteInfoHeaders[key] = value
}

func (t *HeaderTransport) GetPersistentHeader(key string) (string, bool) {
	v, ok := t.persistentWriteInfoHeaders[key]
	return v, ok
}

func (t *HeaderTransport) GetPersistentHeaders() map[string]string {
	res := map[string]string{}
	for k, v := range t.persistentWriteInfoHeaders {
		res[k] = v
	}
	return res
}

func (t *HeaderTransport) ClearPersistentHeaders() {
	if len(t.persistentWriteInfoHeaders) != 0 {
		t.persistentWriteInfoHeaders = map[string]string{}
	}
}

func (t *HeaderTransport) SetHeader(key, value string) {
	t.writeInfoHeaders[key] = value
}

func (t *HeaderTransport) Header(key string) (string, bool) {
	v, ok := t.writeInfoHeaders[key]
	return v, ok
}

func (t *HeaderTransport) Headers() map[string]string {
	res := map[string]string{}
	for k, v := range t.writeInfoHeaders {
		res[k] = v
	}
	return res
}

func (t *HeaderTransport) ClearHeaders() {
	if len(t.writeInfoHeaders) != 0 {
		t.writeInfoHeaders = map[string]string{}
	}
}

func (t *HeaderTransport) ReadHeader(key string) (string, bool) {
	if t.readHeader == nil {
		return "", false
	}
	// per the C++ implementation, prefer persistent headers
	if v, ok := t.readHeader.pHeaders[key]; ok {
		return v, ok
	}
	v, ok := t.readHeader.headers[key]
	return v, ok
}

func (t *HeaderTransport) ReadHeaders() map[string]string {
	res := map[string]string{}
	if t.readHeader == nil {
		return res
	}
	for k, v := range t.readHeader.headers {
		res[k] = v
	}
	for k, v := range t.readHeader.pHeaders {
		res[k] = v
	}
	return res
}

func (t *HeaderTransport) ProtocolID() ProtocolID {
	return t.protoID
}

func (t *HeaderTransport) SetProtocolID(protoID ProtocolID) error {
	if !(protoID == ProtocolIDBinary || protoID == ProtocolIDCompact) {
		return NewTransportException(
			NOT_IMPLEMENTED, fmt.Sprintf("unimplemented proto ID: %s (%#x)", protoID.String(), int64(protoID)),
		)
	}
	t.protoID = protoID
	return nil
}

func (t *HeaderTransport) AddTransform(trans TransformID) error {
	if sup, ok := supportedTransforms[trans]; !ok || !sup {
		return NewTransportException(
			NOT_IMPLEMENTED, fmt.Sprintf("unimplemented transform ID: %s (%#x)", trans.String(), int64(trans)),
		)
	}
	for _, t := range t.writeTransforms {
		if t == trans {
			return nil
		}
	}
	t.writeTransforms = append(t.writeTransforms, trans)
	return nil
}

// applyUntransform fully reads the frame and untransforms into a local buffer
// we need to know the full size of the untransformed data
func (t *HeaderTransport) applyUntransform() error {
	out, err := ioutil.ReadAll(t.framebuf)
	if err != nil {
		return err
	}
	t.frameSize = uint64(len(out))
	t.framebuf = newLimitedByteReader(bytes.NewBuffer(out), int64(len(out)))
	return nil
}

// GetFlags returns the header flags.
func (t *HeaderTransport) GetFlags() HeaderFlags {
	return HeaderFlags(t.flags)
}

// SetFlags sets the header flags.
func (t *HeaderTransport) SetFlags(flags HeaderFlags) {
	t.flags = uint16(flags)
}

// ResetProtocol needs to be called between every frame receive (BeginMessageRead)
// We do this to read out the header for each frame. This contains the length of the
// frame and protocol / metadata info.
func (t *HeaderTransport) ResetProtocol() error {
	t.readHeader = nil
	// TODO(carlverge): We should probably just read in the whole
	// frame here. A bit of extra memory, probably a lot less CPU.
	// Needs benchmark to test.

	hdr := &tHeader{}
	// Consume the header from the input stream
	err := hdr.Read(t.rbuf)
	if err != nil {
		return NewTransportExceptionFromError(err)
	}

	// Set new header
	t.readHeader = hdr
	// Adopt the client's protocol
	t.protoID = hdr.protoID
	t.clientType = hdr.clientType
	t.readSeqID = hdr.seq
	t.flags = hdr.flags

	// Make sure we can't read past the current frame length
	t.frameSize = hdr.payloadLen
	t.framebuf = newLimitedByteReader(t.rbuf, int64(hdr.payloadLen))

	for _, trans := range hdr.transforms {
		xformer, terr := trans.Untransformer()
		if terr != nil {
			return NewTransportExceptionFromError(terr)
		}

		t.framebuf, terr = xformer(t.framebuf)
		if terr != nil {
			return NewTransportExceptionFromError(terr)
		}
	}

	// Fully read the frame and apply untransforms if we have them
	if len(hdr.transforms) > 0 {
		err = t.applyUntransform()
		if err != nil {
			return NewTransportExceptionFromError(err)
		}
	}

	// respond in kind with the client's transforms
	t.writeTransforms = hdr.transforms

	return nil
}

// Open opens the internal transport
func (t *HeaderTransport) Open() error {
	return t.transport.Open()
}

// IsOpen returns whether the current transport is open
func (t *HeaderTransport) IsOpen() bool {
	return t.transport.IsOpen()
}

// Close closes the internal transport
func (t *HeaderTransport) Close() error {
	return t.transport.Close()
}

// UnderlyingTransport gets the underlying transport
func (t *HeaderTransport) UnderlyingTransport() Transport {
	return t.transport
}

// Read reads from the current framebuffer. EOF if the frame is done.
func (t *HeaderTransport) Read(buf []byte) (int, error) {
	n, err := t.framebuf.Read(buf)
	// Shouldn't be possibe, but just in case the frame size was flubbed
	if uint64(n) > t.frameSize {
		n = int(t.frameSize)
	}
	t.frameSize -= uint64(n)
	return n, err
}

// ReadByte reads a single byte from the current framebuffer. EOF if the frame is done.
func (t *HeaderTransport) ReadByte() (byte, error) {
	b, err := t.framebuf.ReadByte()
	t.frameSize--
	return b, err
}

// Write writes multiple bytes to the framebuffer, does not send to transport.
func (t *HeaderTransport) Write(buf []byte) (int, error) {
	n, err := t.wbuf.Write(buf)
	return n, NewTransportExceptionFromError(err)
}

// WriteByte writes a single byte to the framebuffer, does not send to transport.
func (t *HeaderTransport) WriteByte(c byte) error {
	err := t.wbuf.WriteByte(c)
	return NewTransportExceptionFromError(err)
}

// WriteString writes a string to the framebuffer, does not send to transport.
func (t *HeaderTransport) WriteString(s string) (int, error) {
	n, err := t.wbuf.WriteString(s)
	return n, NewTransportExceptionFromError(err)
}

// RemainingBytes returns how many bytes remain in the current recv framebuffer.
func (t *HeaderTransport) RemainingBytes() uint64 {
	return t.frameSize
}

func applyTransforms(buf *bytes.Buffer, transforms []TransformID) (*bytes.Buffer, error) {
	if len(transforms) == 0 {
		return buf, nil
	}

	tmpbuf := bytes.NewBuffer(nil)
	for _, trans := range transforms {
		switch trans {
		case TransformZlib:
			zwr := zlib.NewWriter(tmpbuf)
			_, err := buf.WriteTo(zwr)
			if err != nil {
				return nil, err
			}
			err = zwr.Close()
			if err != nil {
				return nil, err
			}
			buf, tmpbuf = tmpbuf, buf
			tmpbuf.Reset()
		case TransformZstd:
			err := zstdWriter(tmpbuf, buf)
			if err != nil {
				return nil, err
			}
			buf, tmpbuf = tmpbuf, buf
			tmpbuf.Reset()
		default:
			return nil, NewTransportException(
				NOT_IMPLEMENTED, fmt.Sprintf("unimplemented transform ID: %s (%#x)", trans.String(), int64(trans)),
			)
		}
	}
	return buf, nil
}

func (t *HeaderTransport) flushHeader() error {
	hdr := tHeader{}
	hdr.headers = t.writeInfoHeaders
	hdr.pHeaders = t.persistentWriteInfoHeaders
	if t.seqIDExplicitlySet {
		t.seqIDExplicitlySet = false
		// seqID is only explicitly set for requests
		if t.firstRequest {
			t.firstRequest = false
			hdr.headers[ClientMetadataHeader] = ClientMetadata
		}
		hdr.seq = t.writeSeqID
	} else {
		hdr.seq = t.readSeqID
	}
	hdr.transforms = t.writeTransforms

	// protoID, clientType, and flags are state taken from what was recently read
	// from ReadMessageBegin which always calls ResetProtocol.
	// this means header protocol clients supporting out of order requests must also not change
	// these fields dynamically between seq ids on a single transport instance.
	// either we enforce that, or we keep around a lookup table of seq id -> these fields.
	hdr.protoID = t.protoID
	hdr.clientType = t.clientType
	hdr.flags = t.flags

	if t.identity != "" {
		hdr.headers[IdentityHeader] = t.identity
		hdr.headers[IDVersionHeader] = IDVersion
	}

	outbuf, err := applyTransforms(t.wbuf, t.writeTransforms)
	if err != nil {
		return NewTransportExceptionFromError(err)
	}
	t.wbuf = outbuf

	hdr.payloadLen = uint64(t.wbuf.Len())
	err = hdr.calcLenFromPayload()
	if err != nil {
		return NewTransportExceptionFromError(err)
	}

	err = hdr.Write(t.transport)
	return NewTransportExceptionFromError(err)
}

func (t *HeaderTransport) flushFramed() error {
	buflen := t.wbuf.Len()
	framesize := uint32(buflen)
	if buflen > int(MaxFrameSize) {
		return NewTransportException(
			INVALID_FRAME_SIZE,
			fmt.Sprintf("cannot send bigframe of size %d", buflen),
		)
	}

	err := binary.Write(t.transport, binary.BigEndian, framesize)
	return NewTransportExceptionFromError(err)
}

func (t *HeaderTransport) Flush() error {
	var err error

	switch t.clientType {
	case HeaderClientType:
		err = t.flushHeader()
	case FramedDeprecated:
		err = t.flushFramed()
	case FramedCompact:
		err = t.flushFramed()
	default:
		t.wbuf.Reset() // reset incase wbuf pointer changes in xform
		return NewTransportException(
			UNKNOWN_TRANSPORT_EXCEPTION,
			fmt.Sprintf("tHeader cannot flush for clientType %s", t.clientType.String()),
		)
	}

	if err != nil {
		t.wbuf.Reset() // reset incase wbuf pointer changes in xform
		return err
	}

	// Writeout the payload
	if t.wbuf.Len() > 0 {
		_, err = t.wbuf.WriteTo(t.transport)
		if err != nil {
			t.wbuf.Reset() // reset on return
			return NewTransportExceptionFromError(err)
		}
	}

	// Remove the non-persistent headers on flush
	t.ClearHeaders()

	err = t.transport.Flush()

	t.wbuf.Reset() // reset incase wbuf pointer changes in xform
	return NewTransportExceptionFromError(err)
}
