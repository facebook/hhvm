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
	"github.com/klauspost/compress/zstd"
)

const (
	zstdTransformSupport = true
)

func zstdRead(rd byteReader) (byteReader, error) {
	zlrd, err := zstd.NewReader(rd)
	if err != nil {
		return nil, err
	}
	return ensureByteReader(zlrd), nil
}

func zstdWriter(tmpbuf *bytes.Buffer, buf *bytes.Buffer) error {
	zwr, err := zstd.NewWriter(tmpbuf)
	if err != nil {
		return err
	}
	if _, err := buf.WriteTo(zwr); err != nil {
		return err
	}
	if err := zwr.Close(); err != nil {
		return err
	}
	return nil
}
