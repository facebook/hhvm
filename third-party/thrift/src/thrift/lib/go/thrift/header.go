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
	"bufio"
	"bytes"
	"compress/zlib"
	"encoding/binary"
	"fmt"
	"io"
)

// Header keys
const (
	IdentityHeader       string = "identity"
	IDVersionHeader      string = "id_version"
	IDVersion            string = "1"
	PriorityHeader       string = "thrift_priority"
	ClientTimeoutHeader  string = "client_timeout"
	QueueTimeoutHeader   string = "queue_timeout"
	ClientMetadataHeader string = "client_metadata"
	ClientMetadata       string = "{\"agent\":\"headertransport.go\"}"
	// Header Magicks
	// 0 and 16th bits must be 0 to differentiate from framed & unframed
	HeaderMagic         uint32 = 0x0FFF0000
	HeaderMask          uint32 = 0xFFFF0000
	FlagsMask           uint32 = 0x0000FFFF
	HTTPServerMagic     uint32 = 0x504F5354 // POST
	HTTPClientMagic     uint32 = 0x48545450 // HTTP
	HTTPGetClientMagic  uint32 = 0x47455420 // GET
	HTTPHeadClientMagic uint32 = 0x48454144 // HEAD
	BigFrameMagic       uint32 = 0x42494746 // BIGF
	MaxFrameSize        uint32 = 0x3FFFFFFF
	CommonHeaderSize    uint64 = 10
	MaxHeaderSize       uint32 = 131071
)

type ClientType int64

const (
	HeaderClientType ClientType = iota
	FramedDeprecated
	UnframedDeprecated
	HTTPServerType
	HTTPClientType
	FramedCompact
	HTTPGetClientType
	UnknownClientType
	UnframedCompactDeprecated
)

func (c ClientType) String() string {
	switch c {
	case HeaderClientType:
		return "Header"
	case FramedDeprecated:
		return "FramedDeprecated"
	case UnframedDeprecated:
		return "UnframedDeprecated"
	case HTTPServerType:
		return "HTTPServer"
	case HTTPClientType:
		return "HTTPClient"
	case FramedCompact:
		return "FramedCompact"
	case HTTPGetClientType:
		return "HTTPGet"
	case UnframedCompactDeprecated:
		return "UnframedCompactDeprecated"
	case UnknownClientType:
		fallthrough
	default:
		return "Unknown"
	}
}

type HeaderFlags uint16

const (
	HeaderFlagSupportOutOfOrder HeaderFlags = 0x01
	HeaderFlagDuplexReverse     HeaderFlags = 0x08
)

type InfoIDType uint32

const (
	InfoIDPadding   InfoIDType = 0
	InfoIDKeyValue  InfoIDType = 1
	InfoIDPKeyValue InfoIDType = 2
)

// TransformID Numerical ID of transform function
type TransformID uint32

const (
	// TransformNone Default null transform
	TransformNone TransformID = 0
	// TransformZlib Apply zlib compression
	TransformZlib TransformID = 1
	// TransformHMAC Deprecated and no longer supported
	TransformHMAC TransformID = 2
	// TransformSnappy Apply snappy compression
	TransformSnappy TransformID = 3
	// TransformQLZ Deprecated and no longer supported
	TransformQLZ TransformID = 4
	// TransformZstd Apply zstd compression
	TransformZstd TransformID = 5
)

func (c TransformID) String() string {
	switch c {
	case TransformNone:
		return "none"
	case TransformZlib:
		return "zlib"
	case TransformHMAC:
		return "hmac"
	case TransformSnappy:
		return "snappy"
	case TransformQLZ:
		return "qlz"
	case TransformZstd:
		return "zstd"
	default:
		return "unknown"
	}
}

var supportedTransforms = map[TransformID]bool{
	TransformNone:   true,
	TransformZlib:   true,
	TransformHMAC:   false,
	TransformSnappy: false,
	TransformQLZ:    false,
	TransformZstd:   zstdTransformSupport,
}

// Untransformer will find a transform function to wrap a reader with to transformed the data.
func (c TransformID) Untransformer() (func(byteReader) (byteReader, error), error) {
	switch c {
	case TransformNone:
		return func(rd byteReader) (byteReader, error) {
			return rd, nil
		}, nil
	case TransformZlib:
		return func(rd byteReader) (byteReader, error) {
			zlrd, err := zlib.NewReader(rd)
			if err != nil {
				return nil, err
			}
			return ensureByteReader(zlrd), nil
		}, nil
	case TransformZstd:
		return zstdRead, nil
	default:
		return nil, NewProtocolExceptionWithType(
			NOT_IMPLEMENTED, fmt.Errorf("Header transform %s not supported", c.String()),
		)
	}
}

type tHeader struct {
	length     uint64
	flags      uint16
	seq        uint32
	headerLen  uint16
	payloadLen uint64

	protoID    ProtocolID
	transforms []TransformID

	// Map to use for headers
	headers  map[string]string
	pHeaders map[string]string

	// clientType Negotiated client type
	clientType ClientType
}

// byteReader Combined interface to expose original ReadByte calls
type byteReader interface {
	io.Reader
	io.ByteReader
}

// ensureByteReader If a reader does not implement ReadByte, wrap it with a
// buffer that can. Needed for most thrift interfaces.
func ensureByteReader(rd io.Reader) byteReader {
	if brr, ok := rd.(byteReader); ok {
		return brr
	}
	return bufio.NewReader(rd)
}

// limitedByteReader Keep the ByteReader interface when wrapping with a limit
type limitedByteReader struct {
	io.LimitedReader
	// Copy of the original interface given to us that implemented ByteReader
	orig byteReader
}

func newLimitedByteReader(rd byteReader, n int64) *limitedByteReader {
	return &limitedByteReader{
		LimitedReader: io.LimitedReader{R: rd, N: n}, orig: rd,
	}
}

func (r *limitedByteReader) ReadByte() (byte, error) {
	if r.N <= 0 {
		return '0', io.EOF
	}
	b, err := r.orig.ReadByte()
	r.N--
	return b, err
}

func readVarString(buf byteReader) (string, error) {
	strlen, err := binary.ReadUvarint(buf)
	if err != nil {
		return "", fmt.Errorf("tHeader: error reading len of kv string: %s", err.Error())
	}

	strbuf := make([]byte, strlen)
	_, err = io.ReadFull(buf, strbuf)
	if err != nil {
		return "", fmt.Errorf("tHeader: error reading kv string: %s", err.Error())
	}
	return string(strbuf), nil
}

// readHeaderMaps Consume a set of key/value pairs from the buffer
func readInfoHeaderSet(buf byteReader) (map[string]string, error) {
	headers := map[string]string{}
	numkvs, err := binary.ReadUvarint(buf)
	if err != nil {
		return nil, fmt.Errorf("tHeader: error reading number of keyvalues: %s", err.Error())
	}

	for i := uint64(0); i < numkvs; i++ {
		key, err := readVarString(buf)
		if err != nil {
			return nil, fmt.Errorf("tHeader: error reading keyvalue key: %s", err.Error())
		}
		val, err := readVarString(buf)
		if err != nil {
			return nil, fmt.Errorf("tHeader: error reading keyvalue val: %s", err.Error())
		}
		headers[key] = val
	}
	return headers, nil
}

// readTransforms Consume a size delimited transform set from the buffer
// If the there is an unknown or unsupported transform we will bail out.
func readTransforms(buf byteReader) ([]TransformID, error) {
	transforms := []TransformID{}

	numtransforms, err := binary.ReadUvarint(buf)
	if err != nil {
		return nil, NewTransportExceptionFromError(
			fmt.Errorf("tHeader: error reading number of transforms: %s", err.Error()),
		)
	}

	// Read transforms
	for i := uint64(0); i < numtransforms; i++ {
		transformID, err := binary.ReadUvarint(buf)
		if err != nil {
			return nil, NewTransportExceptionFromError(
				fmt.Errorf("tHeader: error reading transforms: %s", err.Error()),
			)
		}
		tid := TransformID(transformID)
		if supported, ok := supportedTransforms[tid]; ok {
			if supported {
				transforms = append(transforms, tid)
			} else {
				return nil, NewTransportExceptionFromError(
					fmt.Errorf("tHeader: unsupported transform: %s", tid.String()),
				)
			}
		} else {
			return nil, NewTransportExceptionFromError(
				fmt.Errorf("tHeader: unknown transform ID: %#x", tid),
			)
		}
	}
	return transforms, nil
}

// readInfoHeaders Read the K/V headers at the end of the header
// This will keep consuming bytes until the buffer returns EOF
func readInfoHeaders(buf byteReader) (map[string]string, map[string]string, error) {
	// var err error
	infoheaders := map[string]string{}
	infopHeaders := map[string]string{}

	for {
		infoID, err := binary.ReadUvarint(buf)

		// this is the last field, read until there is no more padding
		if err == io.EOF {
			break
		}

		if err != nil {
			return nil, nil, NewTransportExceptionFromError(
				fmt.Errorf("tHeader: error reading infoID: %s", err.Error()),
			)
		}

		switch InfoIDType(infoID) {
		case InfoIDPadding:
			continue
		case InfoIDKeyValue:
			hdrs, err := readInfoHeaderSet(buf)
			if err != nil {
				return nil, nil, err
			}
			for k, v := range hdrs {
				infoheaders[k] = v
			}
		case InfoIDPKeyValue:
			hdrs, err := readInfoHeaderSet(buf)
			if err != nil {
				return nil, nil, err
			}
			for k, v := range hdrs {
				infopHeaders[k] = v
			}
		default:
			return nil, nil, NewTransportExceptionFromError(
				fmt.Errorf("tHeader: error reading infoIDType: %#x", infoID),
			)
		}
	}
	return infoheaders, infopHeaders, nil
}

// readVarHeader Read the variable-length trailing header
func (hdr *tHeader) readVarHeader(buf byteReader) error {
	// Read protocol ID
	protoID, err := binary.ReadUvarint(buf)
	if err != nil {
		return NewTransportExceptionFromError(
			fmt.Errorf("tHeader: error reading protocol ID: %s", err.Error()),
		)
	}
	hdr.protoID = ProtocolID(protoID)
	hdr.transforms, err = readTransforms(buf)
	if err != nil {
		return err
	}

	hdr.headers, hdr.pHeaders, err = readInfoHeaders(buf)
	if err != nil {
		return err
	}

	return nil
}

// isCompactFramed Check if the magic value corresponds to compact proto
func isCompactFramed(magic uint32) bool {
	protocolID := int8(magic >> 24)
	protocolVersion := int8((magic >> 16) & uint32(COMPACT_VERSION_MASK))
	return uint8(protocolID) == uint8(COMPACT_PROTOCOL_ID) && (protocolVersion == int8(COMPACT_VERSION) ||
		protocolVersion == int8(COMPACT_VERSION_BE))
}

// analyzeFirst32Bit Guess client type from the first 4 bytes
func analyzeFirst32Bit(word uint32) ClientType {
	if (word & BinaryVersionMask) == BinaryVersion1 {
		return UnframedDeprecated
	} else if isCompactFramed(word) {
		return UnframedCompactDeprecated
	} else if word == HTTPServerMagic ||
		word == HTTPGetClientMagic ||
		word == HTTPHeadClientMagic {
		return HTTPServerType
	} else if word == HTTPClientMagic {
		return HTTPClientType
	}
	return UnknownClientType
}

// analyzeSecond32Bit Find the header client type from the 4-8th bytes of header
func analyzeSecond32Bit(word uint32) ClientType {
	if (word & BinaryVersionMask) == BinaryVersion1 {
		return FramedDeprecated
	}
	if isCompactFramed(word) {
		return FramedCompact
	}
	if (word & HeaderMask) == HeaderMagic {
		return HeaderClientType
	}
	return UnknownClientType
}

// checkFramed If the client type is framed, set appropriate protocolID in
// the header. Otherwise, return an unknown transport error.
func checkFramed(hdr *tHeader, clientType ClientType) error {
	switch clientType {
	case FramedDeprecated:
		hdr.protoID = ProtocolIDBinary
		hdr.clientType = clientType
		hdr.payloadLen = hdr.length
		return nil
	case FramedCompact:
		hdr.protoID = ProtocolIDCompact
		hdr.clientType = clientType
		hdr.payloadLen = hdr.length
		return nil
	default:
		return NewProtocolExceptionWithType(
			NOT_IMPLEMENTED, fmt.Errorf("Transport %s not supported on tHeader", clientType),
		)
	}
}

// readHeaderInfo Consume header information from the buffer
func (hdr *tHeader) Read(buf *bufio.Reader) error {
	var (
		err        error
		firstword  uint32
		secondword uint32
		wordbuf    []byte
	)

	if wordbuf, err = buf.Peek(4); err != nil {
		return NewTransportExceptionFromError(err)
	}
	firstword = binary.BigEndian.Uint32(wordbuf)

	// Check the first word if it matches http/unframed signatures
	// We don't support non-framed protocols, so bail out
	switch clientType := analyzeFirst32Bit(firstword); clientType {
	case UnknownClientType:
		break
	default:
		return NewTransportExceptionFromError(
			fmt.Errorf("Transport %s not supported on tHeader (word=%#x)", clientType, firstword),
		)
	}

	// From here on out, all protocols supported are frame-based. First word is length.
	hdr.length = uint64(firstword)
	if firstword > MaxFrameSize {
		return NewTransportExceptionFromError(
			fmt.Errorf("BigFrames not supported: got size %d", firstword),
		)
	}

	// First word is always length, discard.
	_, err = buf.Discard(4)
	if err != nil {
		// Shouldn't be possible to fail here, but check anyways
		return NewTransportExceptionFromError(err)
	}

	// Only peek here. If it was framed transport, we are now reading payload.
	if wordbuf, err = buf.Peek(4); err != nil {
		return NewTransportExceptionFromError(err)
	}
	secondword = binary.BigEndian.Uint32(wordbuf)

	// Check if we can detect a framed proto, and bail out if we do.
	if clientType := analyzeSecond32Bit(secondword); clientType != HeaderClientType {
		return checkFramed(hdr, clientType)
	}

	// It was not framed proto, assume header and discard that word.
	_, err = buf.Discard(4)
	if err != nil {
		// Shouldn't be possible to fail here, but check anyways
		return NewTransportExceptionFromError(err)
	}

	// Assume header protocol from here on in, parse rest of header
	hdr.flags = uint16(secondword & FlagsMask)
	err = binary.Read(buf, binary.BigEndian, &hdr.seq)
	if err != nil {
		return NewTransportExceptionFromError(err)
	}

	err = binary.Read(buf, binary.BigEndian, &hdr.headerLen)
	if err != nil {
		return NewTransportExceptionFromError(err)
	}

	if uint32(hdr.headerLen*4) > MaxHeaderSize {
		return NewTransportExceptionFromError(
			fmt.Errorf("invalid header length: %d", int64(hdr.headerLen*4)),
		)
	}

	// The length of the payload without the header (fixed is 10)
	hdr.payloadLen = hdr.length - 10 - uint64(hdr.headerLen*4)

	// Limit the reader for the header so we can't overrun
	limbuf := newLimitedByteReader(buf, int64(hdr.headerLen*4))
	hdr.clientType = HeaderClientType
	return hdr.readVarHeader(limbuf)
}

func writeTransforms(transforms []TransformID, buf io.Writer) (int, error) {
	size := 0
	n, err := writeUvarint(uint64(len(transforms)), buf)
	size += n
	if err != nil {
		return size, err
	}

	if transforms == nil {
		return size, nil
	}

	for _, trans := range transforms {
		// FIXME: We should only write supported xforms
		n, err = writeUvarint(uint64(trans), buf)
		size += n
		if err != nil {
			return size, err
		}
	}
	return size, nil
}

func writeUvarint(v uint64, buf io.Writer) (int, error) {
	var b [10]byte
	n := binary.PutUvarint(b[:], v)
	return buf.Write(b[:n])
}

func writeVarString(s string, buf io.Writer) (int, error) {
	n, err := writeUvarint(uint64(len(s)), buf)
	if err != nil {
		return n, err
	}
	n2, err := buf.Write([]byte(s))
	return n + n2, err
}

func writeInfoHeaders(headers map[string]string, infoidtype InfoIDType, buf io.Writer) (int, error) {
	cnt := len(headers)
	size := 0
	if cnt < 1 {
		return 0, nil
	}

	n, err := writeUvarint(uint64(infoidtype), buf)
	size += n
	if err != nil {
		return 0, err
	}

	n, err = writeUvarint(uint64(cnt), buf)
	size += n
	if err != nil {
		return 0, err
	}

	for k, v := range headers {
		n, err = writeVarString(k, buf)
		size += n
		if err != nil {
			return 0, err
		}

		n, err = writeVarString(v, buf)
		size += n
		if err != nil {
			return 0, err
		}
	}

	return size, nil
}

func (hdr *tHeader) writeVarHeader(buf io.Writer) (int, error) {
	size := 0
	n, err := writeUvarint(uint64(hdr.protoID), buf)
	size += n
	if err != nil {
		return size, err
	}

	n, err = writeTransforms(hdr.transforms, buf)
	size += n
	if err != nil {
		return size, err
	}

	n, err = writeInfoHeaders(hdr.pHeaders, InfoIDPKeyValue, buf)
	size += n
	if err != nil {
		return size, err
	}

	n, err = writeInfoHeaders(hdr.headers, InfoIDKeyValue, buf)
	size += n
	if err != nil {
		return size, err
	}

	padding := (4 - size%4) % 4
	for i := 0; i < padding; i++ {
		buf.Write([]byte{byte(0)})
		size++
	}

	return size, err
}

func (hdr *tHeader) calcLenFromPayload() error {
	fixedlen := uint64(0)
	switch hdr.clientType {
	case FramedCompact:
		hdr.length = hdr.payloadLen
		return nil
	case FramedDeprecated:
		hdr.length = hdr.payloadLen
		return nil
	case HeaderClientType:
		// TODO: Changes with bigframes
		fixedlen = 10
	default:
		return NewApplicationException(
			UNKNOWN_TRANSPORT_EXCEPTION,
			fmt.Sprintf("cannot get length of non-framed transport %s", hdr.clientType.String()),
		)
	}
	framesize := uint64(hdr.payloadLen + fixedlen + uint64(hdr.headerLen)*4)
	// FIXME: support bigframes
	if framesize > uint64(MaxFrameSize) {
		return NewTransportException(
			INVALID_FRAME_SIZE,
			fmt.Sprintf("cannot send bigframe of size %d", framesize),
		)
	}
	hdr.length = framesize
	return nil
}

// Write Write out the header, requires payloadLen be set.
func (hdr *tHeader) Write(buf io.Writer) error {
	// Make a reasonably sized temp buffer for the variable header
	hdrbuf := bytes.NewBuffer(nil)
	_, err := hdr.writeVarHeader(hdrbuf)
	if err != nil {
		return err
	}

	if (hdrbuf.Len() % 4) > 0 {
		return NewTransportException(
			INVALID_FRAME_SIZE, fmt.Sprintf("unable to write header of size %d (must be multiple of 4)", hdr.headerLen),
		)
	}
	if hdrbuf.Len() > int(MaxHeaderSize) {
		return NewApplicationException(
			INVALID_FRAME_SIZE, fmt.Sprintf("unable to write header of size %d (max is %d)", hdrbuf.Len(), MaxHeaderSize),
		)
	}
	hdr.headerLen = uint16(hdrbuf.Len() / 4)

	err = hdr.calcLenFromPayload()
	if err != nil {
		return err
	}

	// FIXME: Bad assumption (no err check), but we should be writing to an in-memory buffer here
	binary.Write(buf, binary.BigEndian, uint32(hdr.length))
	binary.Write(buf, binary.BigEndian, uint16(HeaderMagic>>16))
	binary.Write(buf, binary.BigEndian, hdr.flags)
	binary.Write(buf, binary.BigEndian, hdr.seq)
	binary.Write(buf, binary.BigEndian, hdr.headerLen)
	hdrbuf.WriteTo(buf)

	return nil
}
