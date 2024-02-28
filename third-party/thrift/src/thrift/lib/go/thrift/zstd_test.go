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
	"io/ioutil"
	"testing"
)

func TestHeaderZstd(t *testing.T) {
	n := 1
	tmb := NewMemoryBuffer()
	trans := NewHeaderTransport(tmb)
	data := []byte("ASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDF")
	uncompressedlen := 30

	err := trans.AddTransform(TransformSnappy)
	if err == nil {
		t.Fatalf("should have failed adding unsupported transform")
	}

	err = trans.AddTransform(TransformZstd)
	if err != nil {
		t.Fatalf("failed to add transform to frame %d: %s", n, err)
	}

	_, err = trans.Write(data)
	if err != nil {
		t.Fatalf("failed to write frame %d: %s", n, err)
	}
	err = trans.Flush()
	if err != nil {
		t.Fatalf("failed to xmit frame %d: %s", n, err)
	}

	err = trans.ResetProtocol()
	if err != nil {
		t.Fatalf("failed to reset proto for frame %d: %s", n, err)
	}

	frame, err := ioutil.ReadAll(trans)
	if err != nil {
		t.Fatalf("failed to read frame %d: %s", n, err)
	}

	if !bytes.Equal(data, frame) {
		t.Fatalf("data sent does not match receieve on frame %d", n)
	}

	// This is a bit of a stupid test, but make sure that the data
	// got changed somehow
	if len(frame) == uncompressedlen {
		t.Fatalf("data sent was not compressed on frame %d", n)
	}
}

func TestZstd(t *testing.T) {
	want := []byte{0x28, 0xb5, 0x2f, 0xfd}
	compressed, err := compressZstd(want)
	if err != nil {
		t.Fatal(err)
	}
	if bytes.Equal(compressed, want) {
		t.Fatal("zstd compression failed")
	}
	got, err := decompressZstd(compressed)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(got, want) {
		t.Fatalf("zstd roundtrip failed: got %v, want %v", got, want)
	}
}
