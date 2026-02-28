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
	"io"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/stretchr/testify/require"
)

const TRANSPORT_BINARY_DATA_SIZE = 4096

var (
	transport_bdata  []byte // test data for writing; same as data
	transport_header map[string]string
)

type WriteFlusher interface {
	io.Writer
	Flush() error
}

func init() {
	transport_bdata = make([]byte, TRANSPORT_BINARY_DATA_SIZE)
	for i := 0; i < TRANSPORT_BINARY_DATA_SIZE; i++ {
		transport_bdata[i] = byte((i + 'a') % 255)
	}
	transport_header = map[string]string{"key": "User-Agent",
		"value": "Mozilla/5.0 (Windows NT 6.2; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1667.0 Safari/537.36"}
}

func TransportTest(t *testing.T, writer WriteFlusher, reader io.Reader) {
	buf := make([]byte, TRANSPORT_BINARY_DATA_SIZE)

	// Special case for header transport -- need to reset protocol on read
	var headerTrans *headerTransport
	if hdr, ok := reader.(*headerTransport); ok {
		headerTrans = hdr
	}

	_, err := writer.Write(transport_bdata)
	require.NoError(t, err)

	err = writer.Flush()
	require.NoError(t, err)

	if headerTrans != nil {
		err = headerTrans.ResetProtocol()
		require.NoError(t, err)
	}
	n, err := io.ReadFull(reader, buf)
	require.NoError(t, err)
	require.Equal(t, TRANSPORT_BINARY_DATA_SIZE, n)

	for k, v := range buf {
		require.Equal(t, transport_bdata[k], v)
	}
	_, err = writer.Write(transport_bdata)
	require.NoError(t, err)

	err = writer.Flush()
	require.NoError(t, err)

	if headerTrans != nil {
		err = headerTrans.ResetProtocol()
		require.NoError(t, err)
	}
	buf = make([]byte, TRANSPORT_BINARY_DATA_SIZE)
	read := 1
	for n = 0; n < TRANSPORT_BINARY_DATA_SIZE && read != 0; {
		read, err = reader.Read(buf[n:])
		require.NoError(t, err)
		n += read
	}
	require.Equal(t, TRANSPORT_BINARY_DATA_SIZE, n)

	for k, v := range buf {
		require.Equal(t, transport_bdata[k], v)
	}
}

func transportHTTPClientTest(t *testing.T, writer WriteFlusher, reader io.Reader) {
	buf := make([]byte, TRANSPORT_BINARY_DATA_SIZE)

	// Need to assert type of Transport to HTTPClient to expose the Setter
	httpWPostTrans := writer.(*httpClient)
	httpWPostTrans.SetHeader(transport_header["key"], transport_header["value"])

	_, err := writer.Write(transport_bdata)
	require.NoError(t, err)

	err = writer.Flush()
	require.NoError(t, err)

	// Need to assert type of Transport to HTTPClient to expose the Getter
	httpRPostTrans := reader.(*httpClient)
	readHeader := httpRPostTrans.GetHeader(transport_header["key"])
	require.Equal(t, transport_header["value"], readHeader)

	n, err := io.ReadFull(reader, buf)
	require.NoError(t, err)
	require.Equal(t, TRANSPORT_BINARY_DATA_SIZE, n)

	for k, v := range buf {
		require.Equal(t, transport_bdata[k], v)
	}
}

func TestIsEOF(t *testing.T) {
	require.True(t, isEOF(io.EOF))
	require.True(t, isEOF(fmt.Errorf("wrapped error: %w", io.EOF)))
	require.True(t, isEOF(types.NewTransportException(END_OF_FILE, "dummy")))
	require.True(t, isEOF(fmt.Errorf("wrapped trasport error: %w", types.NewTransportException(END_OF_FILE, "dummy"))))
	require.False(t, isEOF(io.ErrNoProgress))
}

func TestString(t *testing.T) {
	require.Equal(t, "TransportIDUnknown", TransportIDUnknown.String())
	require.Equal(t, "TransportIDHeader", TransportIDHeader.String())
	require.Equal(t, "TransportIDRocket", TransportIDRocket.String())
	require.Equal(t, "TransportIDUpgradeToRocket", TransportIDUpgradeToRocket.String())
	require.Equal(t, "UNKNOWN", TransportID(12345).String())
}
