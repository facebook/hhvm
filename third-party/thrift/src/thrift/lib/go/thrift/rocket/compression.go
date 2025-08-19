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

package rocket

import (
	"bytes"
	"compress/zlib"
	"io"

	"github.com/klauspost/compress/zstd"
)

// ## Request Compression
// Requests may be compressed by the client after they have been serialized as described in #Request-serialization. If the request was compressed, the compression algorithm must be specified in the request metadata.
// Reference: https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/server/interface/#request-compression
func compressZstd(data []byte) ([]byte, error) {
	return compressZstdWithLevel(data, zstd.SpeedDefault)
}

func compressZstdLess(data []byte) ([]byte, error) {
	return compressZstdWithLevel(data, zstd.SpeedFastest)
}

func compressZstdMore(data []byte) ([]byte, error) {
	return compressZstdWithLevel(data, zstd.SpeedBestCompression)
}

func compressZstdWithLevel(data []byte, level zstd.EncoderLevel) ([]byte, error) {
	z, err := zstd.NewWriter(nil, zstd.WithEncoderLevel(level), zstd.WithEncoderConcurrency(1))
	if err != nil {
		return nil, err
	}
	defer z.Close()
	return z.EncodeAll(data, nil), nil
}

func decompressZstd(data []byte) ([]byte, error) {
	z, err := zstd.NewReader(nil, zstd.WithDecoderConcurrency(1))
	if err != nil {
		return nil, err
	}
	defer z.Close()
	return z.DecodeAll(data, nil)
}

func compressZlib(data []byte) ([]byte, error) {
	return compressZlibWithLevel(data, zlib.DefaultCompression)
}

func compressZlibLess(data []byte) ([]byte, error) {
	return compressZlibWithLevel(data, zlib.BestSpeed)
}

func compressZlibMore(data []byte) ([]byte, error) {
	return compressZlibWithLevel(data, zlib.BestCompression)
}

func compressZlibWithLevel(data []byte, level int) ([]byte, error) {
	var compressedBuffer bytes.Buffer
	zlibWriter, err := zlib.NewWriterLevel(&compressedBuffer, level)
	if err != nil {
		return nil, err
	}
	_, err = zlibWriter.Write(data)
	if err != nil {
		return nil, err
	}
	err = zlibWriter.Close()
	if err != nil {
		return nil, err
	}
	return compressedBuffer.Bytes(), nil
}

func decompressZlib(compressedData []byte) ([]byte, error) {
	bytesReader := bytes.NewReader(compressedData)
	zlibReader, err := zlib.NewReader(bytesReader)
	if err != nil {
		return nil, err
	}
	defer zlibReader.Close()

	var decompressedBuffer bytes.Buffer
	_, err = io.Copy(&decompressedBuffer, zlibReader)
	if err != nil {
		return nil, err
	}
	return decompressedBuffer.Bytes(), nil
}
