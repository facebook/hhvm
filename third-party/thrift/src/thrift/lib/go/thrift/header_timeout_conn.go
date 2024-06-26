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

type connTimeout struct {
	net.Conn
	readTimeout  time.Duration
	writeTimeout time.Duration
}

var _ net.Conn = (*connTimeout)(nil)

func (c *connTimeout) SetDeadline(t time.Time) error {
	if c.readTimeout > 0 || c.writeTimeout > 0 {
		// timeout is preferred over deadline
		return nil
	}
	return c.Conn.SetDeadline(t)
}

func (c *connTimeout) SetReadDeadline(t time.Time) error {
	if c.readTimeout > 0 {
		// timeout is preferred over deadline
		return nil
	}
	return c.Conn.SetReadDeadline(t)
}

func (c *connTimeout) SetWriteDeadline(t time.Time) error {
	if c.writeTimeout > 0 {
		// timeout is preferred over deadline
		return nil
	}
	return c.Conn.SetWriteDeadline(t)
}

func (c *connTimeout) Read(buf []byte) (int, error) {
	var t time.Time
	if c.readTimeout > 0 {
		t = time.Now().Add(c.readTimeout)
	}
	if err := c.Conn.SetReadDeadline(t); err != nil {
		return 0, err
	}
	return c.Conn.Read(buf)
}

func (c *connTimeout) Write(buf []byte) (int, error) {
	var t time.Time
	if c.writeTimeout > 0 {
		t = time.Now().Add(c.writeTimeout)
	}
	if err := c.Conn.SetWriteDeadline(t); err != nil {
		return 0, err
	}
	return c.Conn.Write(buf)
}
