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
	"encoding/hex"
	"fmt"
	"io"
	"strings"
	"unicode"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/format"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/klauspost/compress/zstd"
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
	// LoadHeaderKey is the header key for thrift headers where ServiceRouter
	// expects load to be reported for cached load.  You must configure this
	// in your SMC tier under the "load_counter" key.
	LoadHeaderKey = "load"
	// ClientTimeoutKey is the header key for thrift headers with ServiceRouter
	ClientTimeoutKey = "client_timeout"
	// ClientID is the header key for thrift headers with ServiceRouter
	ClientID = "client_id"
	// Header Magicks
	// 0 and 16th bits must be 0 to differentiate from framed & unframed
	HeaderMagic      uint32 = 0x0FFF0000
	HeaderMask       uint32 = 0xFFFF0000
	FlagsMask        uint32 = 0x0000FFFF
	HTTPMagicCONNECT uint32 = 0x434F4E4E // CONNect
	HTTPMagicDELETE  uint32 = 0x44454554 // DELEte
	HTTPMagicGET     uint32 = 0x47455420 // GET
	HTTPMagicHEAD    uint32 = 0x48454144 // HEAD
	HTTPMagicOPTIONS uint32 = 0x4F505449 // OPTIons
	HTTPMagicPATCH   uint32 = 0x50415448 // PATCh
	HTTPMagicPOST    uint32 = 0x504F5354 // POST
	HTTPMagicPUT     uint32 = 0x50555420 // PUT
	HTTPMagicTRACE   uint32 = 0x54524145 // TRACe
	HTTPClientMagic  uint32 = 0x48545450 // HTTP
	BigFrameMagic    uint32 = 0x42494746 // BIGF
	MaxFrameSize     uint32 = 0x3FFFFFFF
	CommonHeaderSize uint64 = 10
	MaxHeaderSize    uint32 = 131071
)

// ClientType holds the type of client that was detected by the code,
// which may be a non-thrift client.
type ClientType int64

const (
	HeaderClientType          ClientType = iota // Second word looks like HeaderMagic
	FramedDeprecated                            // Second word looks like BinaryVersion1
	UnframedDeprecated                          // First word looks like BinaryVersion1
	HTTPAttempt                                 // First word looks like HTTPMagicPOST aka POST
	FramedCompact                               // Second word looks like COMPACT_PROTOCOL_ID && (COMPACT_VERSION || COMPACT_VERSION_BE)
	UnknownClientType                           // First and second word don't match any of the magic values.
	UnframedCompactDeprecated                   // First word looks like COMPACT_PROTOCOL_ID && (COMPACT_VERSION || COMPACT_VERSION_BE)
)

func (c ClientType) String() string {
	switch c {
	case HeaderClientType:
		return "Header"
	case FramedDeprecated:
		return "FramedDeprecated"
	case UnframedDeprecated:
		return "UnframedDeprecated"
	case HTTPAttempt:
		return "HTTPAttempt"
	case FramedCompact:
		return "FramedCompact"
	case UnframedCompactDeprecated:
		return "UnframedCompactDeprecated"
	case UnknownClientType:
		return "UnknownClient"
	default:
		return fmt.Sprintf("Unknown (%d)", c)
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
	TransformZstd:   true,
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
		return func(rd byteReader) (byteReader, error) {
			zlrd, err := zstd.NewReader(rd)
			if err != nil {
				return nil, err
			}
			return ensureByteReader(zlrd), nil
		}, nil
	default:
		return nil, types.NewProtocolExceptionWithType(
			types.NOT_IMPLEMENTED, fmt.Errorf("Header transform %s not supported", c.String()),
		)
	}
}

type tHeader struct {
	length     uint64
	flags      uint16
	seq        uint32
	headerLen  uint16
	payloadLen uint64

	protoID    types.ProtocolID
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
func ensureByteReader(reader io.Reader) byteReader {
	if brr, ok := reader.(byteReader); ok {
		return brr
	}
	return bufio.NewReader(reader)
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
		return "", fmt.Errorf("tHeader: error reading len of kv string: %w", err)
	}

	strbuf := make([]byte, strlen)
	_, err = io.ReadFull(buf, strbuf)
	if err != nil {
		return "", fmt.Errorf("tHeader: error reading kv string: %w", err)
	}
	return string(strbuf), nil
}

// readHeaderMaps Consume a set of key/value pairs from the buffer
func readInfoHeaderSet(buf byteReader) (map[string]string, error) {
	headers := map[string]string{}
	numkvs, err := binary.ReadUvarint(buf)
	if err != nil {
		return nil, fmt.Errorf("tHeader: error reading number of keyvalues: %w", err)
	}

	for i := uint64(0); i < numkvs; i++ {
		key, err := readVarString(buf)
		if err != nil {
			return nil, fmt.Errorf("tHeader: error reading keyvalue key: %w", err)
		}
		val, err := readVarString(buf)
		if err != nil {
			return nil, fmt.Errorf("tHeader: error reading keyvalue val: %w", err)
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
		return nil, types.NewTransportExceptionFromError(
			fmt.Errorf("tHeader: error reading number of transforms: %w", err),
		)
	}

	// Read transforms
	for i := uint64(0); i < numtransforms; i++ {
		transformID, err := binary.ReadUvarint(buf)
		if err != nil {
			return nil, types.NewTransportExceptionFromError(
				fmt.Errorf("tHeader: error reading transforms: %w", err),
			)
		}
		tid := TransformID(transformID)
		if supported, ok := supportedTransforms[tid]; ok {
			if supported {
				transforms = append(transforms, tid)
			} else {
				return nil, types.NewTransportExceptionFromError(
					fmt.Errorf("tHeader: unsupported transform: %s", tid.String()),
				)
			}
		} else {
			return nil, types.NewTransportExceptionFromError(
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
			return nil, nil, types.NewTransportExceptionFromError(
				fmt.Errorf("tHeader: error reading infoID: %w", err),
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
			return nil, nil, types.NewTransportExceptionFromError(
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
		return types.NewTransportExceptionFromError(
			fmt.Errorf("tHeader: error reading protocol ID: %w", err),
		)
	}
	hdr.protoID = types.ProtocolID(protoID)
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

// analyzeFirst32Bit Guess client type from the first 4 bytes
func analyzeFirst32Bit(word uint32) ClientType {
	if format.IsBinaryFramed(word) {
		return UnframedDeprecated
	} else if format.IsCompactFramed(word) {
		return UnframedCompactDeprecated
	} else if word == HTTPMagicCONNECT ||
		word == HTTPMagicDELETE ||
		word == HTTPMagicGET ||
		word == HTTPMagicHEAD ||
		word == HTTPMagicOPTIONS ||
		word == HTTPMagicPATCH ||
		word == HTTPMagicPOST ||
		word == HTTPMagicPUT ||
		word == HTTPMagicTRACE ||
		word == HTTPClientMagic {
		return HTTPAttempt
	}
	return UnknownClientType
}

// analyzeSecond32Bit Find the header client type from the 4-8th bytes of header
func analyzeSecond32Bit(word uint32) ClientType {
	if format.IsBinaryFramed(word) {
		return FramedDeprecated
	}
	if format.IsCompactFramed(word) {
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
		hdr.protoID = types.ProtocolIDBinary
		hdr.clientType = clientType
		hdr.payloadLen = hdr.length
		return nil
	case FramedCompact:
		hdr.protoID = types.ProtocolIDCompact
		hdr.clientType = clientType
		hdr.payloadLen = hdr.length
		return nil
	default:
		return fmt.Errorf("Transport %s not supported on tHeader", clientType)
	}
}

func safeStringWithHex(data []byte) string {
	hex := hex.EncodeToString(data)
	clean := strings.Map(func(r rune) rune {
		if unicode.IsPrint(r) {
			return r
		}
		return -1
	}, string(data))
	return fmt.Sprintf("'%s' (%s)", clean, hex)
}

// readHeaderInfo Consume header information from the buffer
func (hdr *tHeader) Read(buf *bufio.Reader) error {
	var (
		err        error
		firstword  uint32
		secondword uint32
		peakbuf    []byte
	)

	if peakbuf, err = buf.Peek(8); err != nil {
		return types.NewTransportExceptionFromError(err)
	}
	firstword = binary.BigEndian.Uint32(peakbuf[0:4])
	secondword = binary.BigEndian.Uint32(peakbuf[4:8])

	// Check the first word if it matches http/unframed signatures
	// We don't support non-framed protocols, so bail out
	switch clientType := analyzeFirst32Bit(firstword); clientType {
	case HTTPAttempt:
		data, _ := buf.Peek(48)
		return types.NewTransportExceptionFromError(
			fmt.Errorf("Possible HTTP connection to thrift port: %s", safeStringWithHex(data)),
		)
	case UnknownClientType:
		break
	default:
		data, _ := buf.Peek(48)
		return types.NewTransportExceptionFromError(
			fmt.Errorf("Transport %s not supported on tHeader: %s", clientType, safeStringWithHex(data)),
		)
	}

	// From here on out, all protocols supported are frame-based. First word is length.
	hdr.length = uint64(firstword)
	if firstword > MaxFrameSize {
		data, _ := buf.Peek(48)
		return types.NewTransportExceptionFromError(
			fmt.Errorf("BigFrames not supported: got size %d: %s", firstword, safeStringWithHex(data)),
		)
	}

	// Check if we can detect a framed proto, and bail out if we do.
	if clientType := analyzeSecond32Bit(secondword); clientType != HeaderClientType {
		if err := checkFramed(hdr, clientType); err != nil {
			data, _ := buf.Peek(48)
			return types.NewProtocolExceptionWithType(
				types.NOT_IMPLEMENTED,
				fmt.Errorf("%w: Possible non-thrift connection: %s", err, safeStringWithHex(data)),
			)
		}
		// This is a valid connection.  Caller expects we've consumed the header.
		_, err = buf.Discard(4)
		if err != nil {
			// Shouldn't be possible to fail here, but check anyways
			return types.NewTransportExceptionFromError(err)
		}
		return nil
	}

	// It was not framed proto, assume header and discard the two words.
	_, err = buf.Discard(8)
	if err != nil {
		// Shouldn't be possible to fail here, but check anyways
		return types.NewTransportExceptionFromError(err)
	}

	// Assume header protocol from here on in, parse rest of header
	hdr.flags = uint16(secondword & FlagsMask)
	err = binary.Read(buf, binary.BigEndian, &hdr.seq)
	if err != nil {
		return types.NewTransportExceptionFromError(err)
	}

	err = binary.Read(buf, binary.BigEndian, &hdr.headerLen)
	if err != nil {
		return types.NewTransportExceptionFromError(err)
	}

	if uint32(hdr.headerLen*4) > MaxHeaderSize {
		return types.NewTransportExceptionFromError(
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

func writeTransforms(transforms []TransformID, writer io.Writer) (int, error) {
	size := 0
	n, err := writeUvarint(uint64(len(transforms)), writer)
	size += n
	if err != nil {
		return size, err
	}

	if transforms == nil {
		return size, nil
	}

	for _, trans := range transforms {
		// FIXME: We should only write supported xforms
		n, err = writeUvarint(uint64(trans), writer)
		size += n
		if err != nil {
			return size, err
		}
	}
	return size, nil
}

func writeUvarint(v uint64, writer io.Writer) (int, error) {
	var b [10]byte
	n := binary.PutUvarint(b[:], v)
	return writer.Write(b[:n])
}

func writeVarString(s string, writer io.Writer) (int, error) {
	n, err := writeUvarint(uint64(len(s)), writer)
	if err != nil {
		return n, err
	}
	n2, err := writer.Write([]byte(s))
	return n + n2, err
}

func writeInfoHeaders(headers map[string]string, infoidtype InfoIDType, writer io.Writer) (int, error) {
	cnt := len(headers)
	size := 0
	if cnt < 1 {
		return 0, nil
	}

	n, err := writeUvarint(uint64(infoidtype), writer)
	size += n
	if err != nil {
		return 0, err
	}

	n, err = writeUvarint(uint64(cnt), writer)
	size += n
	if err != nil {
		return 0, err
	}

	for k, v := range headers {
		n, err = writeVarString(k, writer)
		size += n
		if err != nil {
			return 0, err
		}

		n, err = writeVarString(v, writer)
		size += n
		if err != nil {
			return 0, err
		}
	}

	return size, nil
}

func (hdr *tHeader) writeVarHeader(writer io.Writer) (int, error) {
	size := 0
	n, err := writeUvarint(uint64(hdr.protoID), writer)
	size += n
	if err != nil {
		return size, err
	}

	n, err = writeTransforms(hdr.transforms, writer)
	size += n
	if err != nil {
		return size, err
	}

	n, err = writeInfoHeaders(hdr.pHeaders, InfoIDPKeyValue, writer)
	size += n
	if err != nil {
		return size, err
	}

	n, err = writeInfoHeaders(hdr.headers, InfoIDKeyValue, writer)
	size += n
	if err != nil {
		return size, err
	}

	padding := 4 - size%4
	for i := 0; i < padding; i++ {
		writer.Write([]byte{byte(0)})
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
		return types.NewApplicationException(
			types.UNKNOWN_TRANSPORT_EXCEPTION,
			fmt.Sprintf("cannot get length of non-framed transport %s", hdr.clientType.String()),
		)
	}
	framesize := uint64(hdr.payloadLen + fixedlen + uint64(hdr.headerLen)*4)
	// FIXME: support bigframes
	if framesize > uint64(MaxFrameSize) {
		return types.NewTransportException(
			types.INVALID_FRAME_SIZE,
			fmt.Sprintf("cannot send bigframe of size %d", framesize),
		)
	}
	hdr.length = framesize
	return nil
}

// Write Write out the header, requires payloadLen be set.
func (hdr *tHeader) Write(writer io.Writer) error {
	// Make a reasonably sized temp buffer for the variable header
	hdrbuf := bytes.NewBuffer(nil)
	_, err := hdr.writeVarHeader(hdrbuf)
	if err != nil {
		return err
	}

	if (hdrbuf.Len() % 4) > 0 {
		return types.NewTransportException(
			types.INVALID_FRAME_SIZE, fmt.Sprintf("unable to write header of size %d (must be multiple of 4)", hdr.headerLen),
		)
	}
	if hdrbuf.Len() > int(MaxHeaderSize) {
		return types.NewApplicationException(
			types.INVALID_FRAME_SIZE, fmt.Sprintf("unable to write header of size %d (max is %d)", hdrbuf.Len(), MaxHeaderSize),
		)
	}
	hdr.headerLen = uint16(hdrbuf.Len() / 4)

	err = hdr.calcLenFromPayload()
	if err != nil {
		return err
	}

	// FIXME: Bad assumption (no err check), but we should be writing to an in-memory buffer here
	binary.Write(writer, binary.BigEndian, uint32(hdr.length))
	binary.Write(writer, binary.BigEndian, uint16(HeaderMagic>>16))
	binary.Write(writer, binary.BigEndian, hdr.flags)
	binary.Write(writer, binary.BigEndian, hdr.seq)
	binary.Write(writer, binary.BigEndian, hdr.headerLen)
	hdrbuf.WriteTo(writer)

	return nil
}
