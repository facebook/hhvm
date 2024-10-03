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
	"encoding/binary"
	"fmt"

	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
	"github.com/rsocket/rsocket-go/payload"
)

func checkRequestSetupMetadata8(pay payload.Payload) error {
	pay = payload.Clone(pay)
	metdataBytes, ok := pay.Metadata()
	if !ok {
		return fmt.Errorf("expected metadata in RequestSetupMetadata")
	}
	if len(metdataBytes) < 4 {
		return fmt.Errorf("expected at least 4 bytes for RequestSetupMetadata")
	}
	// read first 4 bytes
	buf := bytes.NewBuffer(metdataBytes[:4])
	var key uint32
	err := binary.Read(buf, binary.BigEndian, &key)
	if err != nil {
		return err
	}
	if int64(key) != rpcmetadata.KRocketProtocolKey {
		return fmt.Errorf("expected key %d, got %d", rpcmetadata.KRocketProtocolKey, key)
	}
	req := rpcmetadata.RequestSetupMetadata{}
	if err := deserializeCompact(metdataBytes[4:], &req); err != nil {
		return err
	}
	minVersion := req.GetMinVersion()
	maxVersion := req.GetMaxVersion()
	if minVersion > 8 || maxVersion < 8 {
		return fmt.Errorf("unsupported version: %d-%d, only version 8 is supported", minVersion, maxVersion)
	}
	return nil
}

func newRequestSetupMetadataVersion8() *rpcmetadata.RequestSetupMetadata {
	res := rpcmetadata.NewRequestSetupMetadata()
	version := int32(8)
	res.SetMaxVersion(&version)
	res.SetMinVersion(&version)
	return res
}

// RSocket setup payload MUST consist of a 32-bit kRocketProtocolKey followed by a compact-serialized RequestSetupMetadata struct.
func newRequestSetupMetadataVersion8Bytes() ([]byte, error) {
	// write key first rpcmetadata.KRocketProtocolKey
	buf := new(bytes.Buffer)
	key := uint32(rpcmetadata.KRocketProtocolKey)
	err := binary.Write(buf, binary.BigEndian, key)
	if err != nil {
		return nil, err
	}
	prefix := buf.Bytes()
	// then write newRequestSetupMetadataVersion8
	serial := NewCompactSerializer()
	bytes, err := serial.Write(newRequestSetupMetadataVersion8())
	if err != nil {
		return nil, err
	}
	return append(prefix, bytes...), nil
}

func newRequestSetupPayloadVersion8() (payload.Payload, error) {
	metadata, err := newRequestSetupMetadataVersion8Bytes()
	if err != nil {
		return nil, err
	}
	return payload.New(nil, metadata), nil
}

// If connection establishment was successful, the server MUST respond with a SetupResponse control message.
func newSetupResponseVersion8() *rpcmetadata.SetupResponse {
	res := rpcmetadata.NewSetupResponse()
	version := int32(8)
	res.SetVersion(&version)
	zstdSupported := true
	res.SetZstdSupported(&zstdSupported)
	return res
}
