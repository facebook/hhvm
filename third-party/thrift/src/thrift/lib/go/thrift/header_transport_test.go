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
	"encoding/hex"
	"io"
	"io/ioutil"
	"testing"
)

func TestHeaderTransport(t *testing.T) {
	trans := NewHeaderTransport(NewMemoryBuffer())
	TransportTest(t, trans, trans)
}

// testHeaderToProto Use a non-header proto to send to a header transport.
// Reflect the result after going through the header pipeline, and make sure that:
// The original proto could still read what it sent
// Header found the correct protocol from the incoming frame
func testHeaderToProto(t *testing.T, clientType ClientType, tmb *MemoryBuffer, proto Format, headertrans *HeaderTransport) {
	tFname, tTypeID, tID, tData := "func", CALL, int32(1), "ASDF"
	err := proto.WriteMessageBegin(tFname, tTypeID, tID)
	if err != nil {
		t.Fatalf("failed to write message for clientType: %s", clientType)
	}
	err = proto.WriteString(tData)
	if err != nil {
		t.Fatalf("failed to write message data for clientType: %s", clientType)
	}
	err = proto.WriteMessageEnd()
	if err != nil {
		t.Fatalf("failed to finalize message for clientType: %s", clientType)
	}
	err = proto.Flush()
	if err != nil {
		t.Fatalf("failed to flush message for clientType: %s", clientType)
	}

	err = headertrans.ResetProtocol()
	if err != nil {
		t.Fatalf(
			"failed to reset protocol for clientType %s: %s (have header %+v)\nGot Frame:\n%s",
			clientType, err, headertrans, hex.Dump(tmb.Bytes()),
		)
	}

	if headertrans.clientType != clientType {
		t.Fatalf(
			"clientType discovered incorrect: expected=%s actual=%s",
			clientType, headertrans.clientType,
		)
	}
	frame, err := ioutil.ReadAll(headertrans)
	if err != nil {
		t.Fatalf("failed to read header transport frame: %s", err)
	}

	// reflect the frame back at the original proto
	_, err = headertrans.Write(frame)
	if err != nil {
		t.Fatalf("failed to write to header transport: %s", err)
	}
	err = headertrans.Flush()
	if err != nil {
		t.Fatalf("failed to flush to header transport: %s", err)
	}

	fname, typeID, seq, err := proto.ReadMessageBegin()
	if err != nil {
		t.Fatalf("failed to read reflected message for clientType %s: %s", clientType, err)
	}
	data, err := proto.ReadString()
	if err != nil {
		t.Fatalf("failed to read reflected data for clientType %s: %s", clientType, err)
	}
	err = proto.ReadMessageEnd()
	if err != nil {
		t.Fatalf("failed to finalize reflected message for clientType %s: %s", clientType, err)
	}

	assertEq(t, tFname, fname)
	assertEq(t, tTypeID, typeID)
	assertEq(t, tID, seq)
	assertEq(t, tData, data)
}

func TestHeaderFramedBinary(t *testing.T) {
	tmb := NewMemoryBuffer()
	testHeaderToProto(
		t, FramedDeprecated, tmb,
		NewBinaryProtocol(NewFramedTransport(tmb), true, true),
		NewHeaderTransport(tmb),
	)
}

func TestHeaderFramedCompact(t *testing.T) {
	tmb := NewMemoryBuffer()
	testHeaderToProto(
		t, FramedCompact, tmb,
		NewCompactProtocol(NewFramedTransport(tmb)),
		NewHeaderTransport(tmb),
	)
}

func TestHeaderProtoID(t *testing.T) {
	n := 1
	tmb := NewMemoryBuffer()
	// write transport
	trans1 := NewHeaderTransport(tmb)
	// read transport
	trans2 := NewHeaderTransport(tmb)
	targetID := ProtocolIDBinary

	assertEq(t, DefaulprotoID, trans1.ProtocolID())

	err := trans1.SetProtocolID(targetID)
	if err != nil {
		t.Fatalf("failed to set binary protocol")
	}

	assertEq(t, targetID, trans1.ProtocolID())

	_, err = trans1.Write([]byte("ASDF"))
	if err != nil {
		t.Fatalf("failed to write frame %d: %s", n, err)
	}
	err = trans1.Flush()
	if err != nil {
		t.Fatalf("failed to xmit frame %d: %s", n, err)
	}

	assertEq(t, DefaulprotoID, trans2.ProtocolID())
	err = trans2.ResetProtocol()
	if err != nil {
		t.Fatalf("failed to reset proto for frame %d: %s", n, err)
	}
	// Make sure the protocol gets changed after recving the frame
	assertEq(t, targetID, trans1.ProtocolID())
}

func TestHeaderHeaders(t *testing.T) {
	n := 1
	tmb := NewMemoryBuffer()
	// write transport
	trans1 := NewHeaderTransport(tmb)
	// read transport
	trans2 := NewHeaderTransport(tmb)

	// make sure we don't barf reading header with no frame
	_, ok := trans1.GetResponseHeader("something")
	assertEq(t, false, ok)
	assertEq(t, 0, len(trans1.GetResponseHeaders()))

	trans1.SetIdentity("localhost")
	trans1.SetRequestHeader("thrift_protocol", "compact")
	trans1.SetRequestHeader("thrift_transport", "header")
	trans1.SetRequestHeader("preferred_cheese", "cheddar")
	trans1.SetPersistentHeader("preferred_cheese", "gouda")

	assertEq(t, 3, len(trans1.Headers()))
	assertEq(t, 1, len(trans1.GetPersistentHeaders()))

	headerval, _ := trans1.GetRequestHeader("preferred_cheese")
	assertEq(t, "cheddar", headerval)
	headerval, _ = trans1.GetPersistentHeader("preferred_cheese")
	assertEq(t, "gouda", headerval)

	_, err := trans1.Write([]byte("ASDF"))
	if err != nil {
		t.Fatalf("failed to write frame %d: %s", n, err)
	}
	err = trans1.Flush()
	if err != nil {
		t.Fatalf("failed to xmit frame %d: %s", n, err)
	}

	// Make sure we zero the headers
	assertEq(t, 0, len(trans1.Headers()))
	// But not the persistent ones
	assertEq(t, 1, len(trans1.GetPersistentHeaders()))

	err = trans2.ResetProtocol()
	if err != nil {
		t.Fatalf("failed to reset proto for frame %d: %s", n, err)
	}

	headerval, _ = trans2.GetResponseHeader("thrift_protocol")
	assertEq(t, "compact", headerval)
	headerval, _ = trans2.GetResponseHeader("thrift_transport")
	assertEq(t, "header", headerval)
	// make sure we prefer persistent headers
	headerval, _ = trans2.GetResponseHeader("preferred_cheese")
	assertEq(t, "gouda", headerval)
	assertEq(t, "localhost", trans2.peerIdentity())
	assertEq(t, 5, len(trans2.GetResponseHeaders()))

	trans2.readHeader.headers[IDVersionHeader] = "invalid"
	assertEq(t, "", trans2.peerIdentity())

	trans1.ClearPersistentHeaders()
	assertEq(t, 0, len(trans1.GetPersistentHeaders()))
}

func TestHeaderRWSmall(t *testing.T) {
	n := 1
	tmb := NewMemoryBuffer()
	trans := NewHeaderTransport(tmb)
	data := []byte("ASDFASDFASDF")

	_, err := trans.Write(data)
	if err != nil {
		t.Fatalf("failed to write frame %d: %s", n, err)
	}

	err = trans.WriteByte('A')
	if err != nil {
		t.Fatalf("failed to writebyte frame %d: %s", n, err)
	}

	_, err = trans.WriteString("SDF")
	if err != nil {
		t.Fatalf("failed to writestring frame %d: %s", n, err)
	}

	err = trans.Flush()
	if err != nil {
		t.Fatalf("failed to xmit frame %d: %s", n, err)
	}

	err = trans.ResetProtocol()
	if err != nil {
		t.Fatalf("failed to reset proto for frame %d: %s", n, err)
	}

	outbuf := make([]byte, len(data))
	read, err := trans.Read(outbuf)
	if err != nil {
		t.Fatalf("failed to read frame %d: %s", n, err)
	}
	assertEq(t, len(data), read)
	if bytes.Compare(data, outbuf) != 0 {
		t.Fatalf("data sent does not match receieve on frame %d", n)
	}

	outb, err := trans.ReadByte()
	if err != nil {
		t.Fatalf("failed to readbyte frame %d: %s", n, err)
	}
	assertEq(t, outb, []byte("A")[0])

	outstr := make([]byte, 3)
	read, err = trans.Read(outstr)
	if err != nil {
		t.Fatalf("failed to read frame %d: %s", n, err)
	}
	assertEq(t, 3, read)
	if bytes.Compare([]byte("SDF"), outstr) != 0 {
		t.Fatalf("data sent does not match receieve on frame %d", n)
	}

	outb, err = trans.ReadByte()
	if err != io.EOF {
		t.Fatalf("frame %d should have been EOF, got: %v (byte=%#x)", n, err, outb)
	}
}

func TestHeaderZlib(t *testing.T) {
	n := 1
	tmb := NewMemoryBuffer()
	trans := NewHeaderTransport(tmb)
	data := []byte("ASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDF")
	uncompressedlen := 30

	err := trans.AddTransform(TransformSnappy)
	if err == nil {
		t.Fatalf("should have failed adding unsupported transform")
	}

	err = trans.AddTransform(TransformZlib)
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

	if bytes.Compare(data, frame) != 0 {
		t.Fatalf("data sent does not match receieve on frame %d", n)
	}

	// This is a bit of a stupid test, but make sure that the data
	// got changed somehow
	if len(frame) == uncompressedlen {
		t.Fatalf("data sent was not compressed on frame %d", n)
	}
}

func testRWOnce(t *testing.T, n int, data []byte, trans *HeaderTransport) {
	_, err := trans.Write(data)
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
		t.Fatalf("failed to recv frame %d: %s", n, err)
	}

	if bytes.Compare(data, frame) != 0 {
		t.Fatalf("data sent does not match receieve on frame %d", n)
	}
}

func TestHeaderTransportRWMultiple(t *testing.T) {
	tmb := NewMemoryBuffer()
	trans := NewHeaderTransport(tmb)

	// Test Junk Data
	testRWOnce(t, 1, []byte("ASDF"), trans)
	// Some sane thrift req/replies
	testRWOnce(t, 2, GetStatusCallData, trans)
	testRWOnce(t, 3, GetStatusReplyData, trans)
}

func BenchmarkHeaderFlush(b *testing.B) {
	data := []byte("ASDF")

	for n := 0; n < b.N; n++ {
		tmb := NewMemoryBuffer()
		trans1 := NewHeaderTransport(tmb)

		trans1.SetIdentity("localhost")
		trans1.SetRequestHeader("thrift_protocol", "compact")
		trans1.SetRequestHeader("thrift_transport", "header")
		trans1.SetRequestHeader("preferred_cheese", "cheddar")
		trans1.SetPersistentHeader("preferred_cheese", "gouda")

		_, err := trans1.Write(data)
		err = trans1.Flush()

		if err != nil {
			b.Fatalf("failed to flush, %s", err)
		}

		trans1.Close()
	}
}
