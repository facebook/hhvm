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
	"net"
	"time"
)

type mockSocket struct {
	Transport
}

var _ Socket = (*mockSocket)(nil)

func newMockSocket() *mockSocket {
	return &mockSocket{
		Transport: NewMemoryBuffer(),
	}
}

func (s *mockSocket) Open() error {
	return nil
}

func (s *mockSocket) Bytes() []byte {
	return s.Transport.(*MemoryBuffer).Bytes()
}

type mockAddr struct {
	addr string
}

func (m *mockAddr) Network() string {
	return "tcp"
}

func (m *mockAddr) String() string {
	return m.addr
}

func (s *mockSocket) LocalAddr() net.Addr {
	return &mockAddr{"<local mock socket>"}
}

func (s *mockSocket) RemoteAddr() net.Addr {
	return &mockAddr{"<remote mock socket>"}
}

func (s *mockSocket) SetDeadline(t time.Time) error {
	return nil
}

func (s *mockSocket) SetReadDeadline(t time.Time) error {
	return nil
}

func (s *mockSocket) SetWriteDeadline(t time.Time) error {
	return nil
}
