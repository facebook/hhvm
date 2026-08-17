//go:build linux

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

// queryPeerCred for Linux returns peer's effective creds for a UDS connection.
// PID/UID/GID are resolved via SO_PEERCRED (ucred). Returns (nil, err) for
// non-uDS transports and on syscall failure.
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
		cred    *unix.Ucred
		credErr error
	)
	ctrlErr := rawConn.Control(func(fd uintptr) {
		sfd := int(fd) // #nosec G115 -- fd is a valid socket descriptor, always a small non-negative int
		cred, credErr = unix.GetsockoptUcred(sfd, unix.SOL_SOCKET, unix.SO_PEERCRED)
	})
	if ctrlErr != nil {
		return nil, fmt.Errorf("peercred: raw conn control: %w", ctrlErr)
	}
	if credErr != nil {
		return nil, fmt.Errorf("peercred: getsockopt SO_PEERCRED: %w", credErr)
	}
	if cred == nil {
		return nil, errors.New("peercred: SO_PEERCRED returned no credentials")
	}

	return &PeerEffectiveCreds{
		PID: cred.Pid,
		UID: cred.Uid,
		GID: cred.Gid,
	}, nil
}
