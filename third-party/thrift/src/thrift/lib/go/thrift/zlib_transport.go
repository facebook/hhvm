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
	"compress/zlib"
	"io"
	"log"
)

// ZlibTransport is a Transport implementation that makes use of zlib compression.
type ZlibTransport struct {
	reader io.ReadCloser
	buffer io.ReadWriteCloser
	writer *zlib.Writer
}

var _ io.ReadWriteCloser = (*ZlibTransport)(nil)

// NewZlibTransport constructs a new instance of ZlibTransport
func NewZlibTransport(trans io.ReadWriteCloser, level int) (*ZlibTransport, error) {
	w, err := zlib.NewWriterLevel(trans, level)
	if err != nil {
		log.Println(err)
		return nil, err
	}

	return &ZlibTransport{
		writer: w,
		buffer: trans,
	}, nil
}

// Close closes the reader and writer (flushing any unwritten data) and closes
// the underlying buffer.
func (z *ZlibTransport) Close() error {
	if z.reader != nil {
		if err := z.reader.Close(); err != nil {
			return err
		}
	}
	if err := z.writer.Close(); err != nil {
		return err
	}
	return z.buffer.Close()
}

// Flush flushes the writer and its underlying buffer.
func (z *ZlibTransport) Flush() error {
	if err := z.writer.Flush(); err != nil {
		return err
	}
	if flusher, ok := z.buffer.(Flusher); ok {
		return flusher.Flush()
	}
	return nil
}

func (z *ZlibTransport) Read(p []byte) (int, error) {
	if z.reader == nil {
		r, err := zlib.NewReader(z.buffer)
		if err != nil {
			return 0, NewTransportExceptionFromError(err)
		}
		z.reader = r
	}

	return z.reader.Read(p)
}

func (z *ZlibTransport) Write(p []byte) (int, error) {
	return z.writer.Write(p)
}
