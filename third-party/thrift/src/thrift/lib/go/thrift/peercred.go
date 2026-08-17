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
	"errors"
	"net"
)

// PeerEffectiveCreds holds the connecting peer's kernel-attested effective
// creds for a UDS conn. On macOS, PID is the effective PID; on Linux there is no distinction.
type PeerEffectiveCreds struct {
	PID int32
	UID uint32
	// GID is 0 if the peer reports no groups (macOS xucred), mirroring C++ cr_gid.
	GID uint32
}

// ErrNoPeerCred indicates that no queryable peer credentials exist on this
// connection (e.g. a non-UDS transport such as TCP, or an unsupported platform
// such as Windows, or a conn that does not expose its underlying UDS socket).
// It reaches ConnInfo.PeerCredError only for conns that are queried at all, as
// non-UDS conns are skipped; callers can use errors.Is to distinguish this
// expected case from a real syscall failure.
var ErrNoPeerCred = errors.New("peer credentials not available for this connection")

// unwrapUnixConn returns the underlying *net.UnixConn for a conn so peer-creds
// can be read from its socket. Unwraps wrappers exposing NetConn (eg. *tls.Conn)
// to reach the underlying socket. Returns (nil, false) for non-UDS transports (eg. TCP)
func unwrapUnixConn(conn net.Conn) (*net.UnixConn, bool) {
	switch c := conn.(type) {
	case *net.UnixConn:
		return c, true
	case interface{ NetConn() net.Conn }:
		return unwrapUnixConn(c.NetConn())
	}
	return nil, false
}
