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
	"testing"
)

func TestSocketIsntListeningAfterInterrupt(t *testing.T) {
	host := "127.0.0.1"
	port := 9090
	addr := fmt.Sprintf("%s:%d", host, port)

	socket, err := NewListener(addr)
	if err != nil {
		t.Fatalf("Failed to create server socket: %s", err)
	}
	socket.Close()

	newSocket, err := NewListener(addr)
	if err != nil {
		t.Fatalf("Failed to create server socket: %s", err)
	}
	defer newSocket.Close()
	if err != nil {
		t.Fatalf("Failed to rebinds: %s", err)
	}
}
