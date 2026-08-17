//go:build linux || darwin

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
	"context"
	"net"
	"os"
	"path/filepath"
	"testing"
	"time"

	"github.com/stretchr/testify/require"
)

// udsConnPair returns a connected Unix-domain-socket pair; both ends live in
// this test process, so the peer credentials are this process's own.
func udsConnPair(t *testing.T) (server, client net.Conn) {
	t.Helper()
	// Not t.TempDir(): it embeds the test name, which can exceed the ~104 byte
	// macOS limit on Unix socket paths.
	dir, err := os.MkdirTemp("", "")
	require.NoError(t, err)
	t.Cleanup(func() { _ = os.RemoveAll(dir) })
	sock := filepath.Join(dir, "s")
	ln, err := net.Listen("unix", sock)
	require.NoError(t, err)
	defer ln.Close()
	// Timeout so that failed call shows instantly and doesn't hang until test times out.
	require.NoError(t, ln.(*net.UnixListener).SetDeadline(time.Now().Add(5*time.Second)))

	type dialResult struct {
		conn net.Conn
		err  error
	}
	dialed := make(chan dialResult, 1)
	go func() {
		c, derr := net.Dial("unix", sock)
		dialed <- dialResult{conn: c, err: derr}
	}()

	server, err = ln.Accept()
	require.NoError(t, err)

	res := <-dialed
	require.NoError(t, res.err)
	require.NotNil(t, res.conn)
	return server, res.conn
}

// queryPeerCred returns this process's own pid/uid over a real UDS pair.
func TestQueryPeerCredUDS(t *testing.T) {
	server, client := udsConnPair(t)
	defer server.Close()
	defer client.Close()

	creds, err := queryPeerCred(server)
	require.NoError(t, err)
	require.NotNil(t, creds)
	require.Equal(t, int32(os.Getpid()), creds.PID)
	require.Equal(t, uint32(os.Getuid()), creds.UID)
}

// queryPeerCred reports an error and nil creds for a non-UDS transport.
func TestQueryPeerCredNonUDS(t *testing.T) {
	c1, c2 := net.Pipe()
	defer c1.Close()
	defer c2.Close()

	creds, err := queryPeerCred(c1)
	require.Error(t, err)
	require.Nil(t, creds)
}

// netConnWrapper mimics *tls.Conn: wraps conn and exposes the underlying
// one via NetConn.
type netConnWrapper struct {
	net.Conn
}

func (w netConnWrapper) NetConn() net.Conn { return w.Conn }

// queryPeerCred unwraps nested NetConn wrappers to reach the underlying socket.
// Reports no creds, should the wrapped conn not be a UDS
func TestQueryPeerCredWrappedConn(t *testing.T) {
	server, client := udsConnPair(t)
	defer server.Close()
	defer client.Close()

	// Wrapped twice, so a single-level unwrap would not reach the socket.
	creds, err := queryPeerCred(netConnWrapper{netConnWrapper{server}})
	require.NoError(t, err)
	require.NotNil(t, creds)
	require.Equal(t, int32(os.Getpid()), creds.PID)
	require.Equal(t, uint32(os.Getuid()), creds.UID)

	c1, c2 := net.Pipe()
	defer c1.Close()
	defer c2.Close()

	creds, err = queryPeerCred(netConnWrapper{c1})
	require.Error(t, err)
	require.Nil(t, creds)
}

// withConnInfo surfaces the peer credentials on ConnInfo for a UDS peer, and
// leaves both creds and error nil for a non-UDS transport, which is not queried.
func TestWithConnInfoPeerCred(t *testing.T) {
	server, client := udsConnPair(t)
	defer server.Close()
	defer client.Close()

	info, ok := connInfoFromContext(withConnInfo(context.Background(), server, nil))
	require.True(t, ok)
	require.NoError(t, info.PeerCredError)
	require.NotNil(t, info.PeerEffectiveCreds)
	require.Equal(t, int32(os.Getpid()), info.PeerEffectiveCreds.PID)
	require.Equal(t, uint32(os.Getuid()), info.PeerEffectiveCreds.UID)

	c1, c2 := net.Pipe()
	defer c1.Close()
	defer c2.Close()

	pipeInfo, ok := connInfoFromContext(withConnInfo(context.Background(), c1, nil))
	require.True(t, ok)
	require.Nil(t, pipeInfo.PeerEffectiveCreds)
	require.NoError(t, pipeInfo.PeerCredError)
}
