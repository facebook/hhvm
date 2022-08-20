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

import "testing"

func TestNewSocket(t *testing.T) {
	socket, err := NewSocket(SocketTimeout(10), SocketAddr("localhost6:1234"))
	if err != nil {
		t.Error(err)
	}
	if socket.timeout != 10 {
		t.Error("wrong timeout")
	}
	if socket.addr.String() != "[::1]:1234" {
		t.Errorf("wrong address: %s", socket.addr)
	}
}
