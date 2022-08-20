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

import "io"

type richTransport struct {
	Transport
}

// Wraps Transport to provide RichTransport interface
func NewRichTransport(trans Transport) RichTransport {
	return &richTransport{trans}
}

func (r *richTransport) ReadByte() (c byte, err error) {
	return readByte(r.Transport)
}

func (r *richTransport) WriteByte(c byte) error {
	return writeByte(r.Transport, c)
}

func (r *richTransport) WriteString(s string) (n int, err error) {
	return r.Write([]byte(s))
}

func (r *richTransport) RemainingBytes() (num_bytes uint64) {
	return r.Transport.RemainingBytes()
}

func readByte(r io.Reader) (c byte, err error) {
	v := [1]byte{0}
	n, err := r.Read(v[0:1])
	if n > 0 && (err == nil || err == io.EOF) {
		return v[0], nil
	}
	if n > 0 && err != nil {
		return v[0], err
	}
	if err != nil {
		return 0, err
	}
	return v[0], nil
}

func writeByte(w io.Writer, c byte) error {
	v := [1]byte{c}
	_, err := w.Write(v[0:1])
	return err
}
