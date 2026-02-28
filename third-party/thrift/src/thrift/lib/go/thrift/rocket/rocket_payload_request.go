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

package rocket

import (
	"context"
	"fmt"
	"maps"
	"math"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
	"github.com/rsocket/rsocket-go/payload"
)

// EncodeRequestPayload encodes a request payload.
func EncodeRequestPayload(
	ctx context.Context,
	name string,
	protoID rpcmetadata.ProtocolId,
	rpcKind rpcmetadata.RpcKind,
	headers map[string]string,
	compression rpcmetadata.CompressionAlgorithm,
	dataBytes []byte,
) (payload.Payload, error) {
	metadata := rpcmetadata.NewRequestRpcMetadata().
		SetName(&name).
		SetProtocol(&protoID).
		SetKind(&rpcKind).
		SetCompression(&compression).
		SetOtherMetadata(maps.Clone(headers))

	frameworkMetadata := getFrameworkMetadata(ctx)
	if frameworkMetadata != nil {
		metadata.SetFrameworkMetadata(frameworkMetadata)
	}

	if rpcOpts := types.GetRPCOptions(ctx); rpcOpts != nil && rpcOpts.QueueTimeout > 0 {
		//nolint:gosec // G115: guarded by max int32
		queueTimeoutMs := int32(min(rpcOpts.QueueTimeout.Milliseconds(), math.MaxInt32))
		metadata.SetQueueTimeoutMs(&queueTimeoutMs)
	}

	if interactionID, ok := types.GetInteractionIDFromContext(ctx); ok {
		if interactionName, ok := types.GetInteractionCreateFromContext(ctx); ok {
			// New interaction (not yet created)
			metadata.SetInteractionCreate(&rpcmetadata.InteractionCreate{
				InteractionId:   interactionID,
				InteractionName: interactionName,
			})
		} else {
			// Existing interaction (already created)
			metadata.SetInteractionId(&interactionID)
		}
	}

	return EncodePayloadMetadataAndData(metadata, dataBytes, compression)
}

func GetRequestRpcMetadataHeaders(metadata *rpcmetadata.RequestRpcMetadata) map[string]string {
	headersMap := make(map[string]string)
	maps.Copy(headersMap, metadata.GetOtherMetadata())
	if metadata.IsSetClientId() {
		headersMap["client_id"] = metadata.GetClientId()
	}
	if metadata.IsSetLoadMetric() {
		headersMap["load"] = metadata.GetLoadMetric()
	}
	if metadata.IsSetClientTimeoutMs() {
		headersMap["client_timeout"] = fmt.Sprintf("%d", metadata.GetClientTimeoutMs())
	}
	if metadata.IsSetServiceTraceMeta() {
		headersMap["service_trace_meta"] = metadata.GetServiceTraceMeta()
	}
	return headersMap
}

func CompressionAlgorithmFromCompressionConfig(compressionConfig *rpcmetadata.CompressionConfig) rpcmetadata.CompressionAlgorithm {
	if compressionConfig == nil {
		return rpcmetadata.CompressionAlgorithm_NONE
	}

	compression := rpcmetadata.CompressionAlgorithm_NONE
	codecConfig := compressionConfig.GetCodecConfig()
	if zlibConfig := codecConfig.GetZlibConfig(); zlibConfig != nil {
		switch zlibConfig.GetLevelPreset() {
		case rpcmetadata.ZlibCompressionLevelPreset_DEFAULT:
			compression = rpcmetadata.CompressionAlgorithm_ZLIB
		case rpcmetadata.ZlibCompressionLevelPreset_LESS:
			compression = rpcmetadata.CompressionAlgorithm_ZLIB_LESS
		case rpcmetadata.ZlibCompressionLevelPreset_MORE:
			compression = rpcmetadata.CompressionAlgorithm_ZLIB_MORE
		default:
			compression = rpcmetadata.CompressionAlgorithm_ZLIB
		}
	} else if zstdConfig := codecConfig.GetZstdConfig(); zstdConfig != nil {
		switch zstdConfig.GetLevelPreset() {
		case rpcmetadata.ZstdCompressionLevelPreset_DEFAULT:
			compression = rpcmetadata.CompressionAlgorithm_ZSTD
		case rpcmetadata.ZstdCompressionLevelPreset_LESS:
			compression = rpcmetadata.CompressionAlgorithm_ZSTD_LESS
		case rpcmetadata.ZstdCompressionLevelPreset_MORE:
			compression = rpcmetadata.CompressionAlgorithm_ZSTD_MORE
		default:
			compression = rpcmetadata.CompressionAlgorithm_ZSTD
		}
	} else if lz4Config := codecConfig.GetLz4Config(); lz4Config != nil {
		switch lz4Config.GetLevelPreset() {
		case rpcmetadata.Lz4CompressionLevelPreset_DEFAULT:
			compression = rpcmetadata.CompressionAlgorithm_LZ4
		case rpcmetadata.Lz4CompressionLevelPreset_LESS:
			compression = rpcmetadata.CompressionAlgorithm_LZ4_LESS
		case rpcmetadata.Lz4CompressionLevelPreset_MORE:
			compression = rpcmetadata.CompressionAlgorithm_LZ4_MORE
		default:
			compression = rpcmetadata.CompressionAlgorithm_LZ4
		}
	} else if customConfig := codecConfig.GetCustomConfig(); customConfig != nil {
		compression = rpcmetadata.CompressionAlgorithm_CUSTOM
	}
	return compression
}
