//go:build darwin

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
	"fmt"
	"net"

	"golang.org/x/sys/unix"
)

// queryPeerCred for MacOS returns peer's effective creds for a UDS connection.
// UID/GID are resolved via LOCAL_PEERCRED (xucred) and PID via LOCAL_PEEREPID.
// Returns (nil, err) for non-UDS transports and on syscall failure.
func queryPeerCred(conn net.Conn) (*PeerEffectiveCreds, error) {
	uc, ok := unwrapUnixConn(conn)
	if !ok {
		return nil, ErrNoPeerCred
	}
	rawConn, err := uc.SyscallConn()
	if err != nil {
		return nil, fmt.Errorf("peercred: syscall conn: %w", err)
	}

	var (
		xucred *unix.Xucred
		epid   int
		optErr error
	)
	ctrlErr := rawConn.Control(func(fd uintptr) {
		sfd := int(fd) // #nosec G115 -- fd is a valid socket descriptor, always a small non-negative int
		xucred, optErr = unix.GetsockoptXucred(sfd, unix.SOL_LOCAL, unix.LOCAL_PEERCRED)
		if optErr != nil {
			return
		}
		epid, optErr = unix.GetsockoptInt(sfd, unix.SOL_LOCAL, unix.LOCAL_PEEREPID)
	})
	if ctrlErr != nil {
		return nil, fmt.Errorf("peercred: raw conn control: %w", ctrlErr)
	}
	if optErr != nil {
		return nil, fmt.Errorf("peercred: getsockopt LOCAL_PEERCRED/PEEREPID: %w", optErr)
	}
	if xucred == nil {
		return nil, errors.New("peercred: LOCAL_PEERCRED returned no credentials")
	}

	var gid uint32
	if xucred.Ngroups > 0 {
		gid = xucred.Groups[0]
	}
	return &PeerEffectiveCreds{
		PID: int32(epid), // #nosec G115 -- process ids fit in int32
		UID: xucred.Uid,
		GID: gid,
	}, nil
}
