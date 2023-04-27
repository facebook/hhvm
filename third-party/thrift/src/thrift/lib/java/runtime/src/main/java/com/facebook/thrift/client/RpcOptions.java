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

package com.facebook.thrift.client;

import com.facebook.thrift.model.CompressionAlgorithm;
import com.facebook.thrift.model.ProtocolId;
import com.facebook.thrift.model.RpcPriority;
import com.google.common.base.Preconditions;
import java.util.HashMap;
import java.util.Map;
import javax.annotation.Nonnull;

public class RpcOptions {

  public static final RpcOptions EMPTY = new RpcOptions.Builder().build();

  private final ProtocolId protocolId;
  private final Integer seqId;
  private final Integer clientTimeoutMs;
  private final Integer queueTimeoutMs;
  private final RpcPriority priority;
  private final Map<String, String> otherMetadata;
  private final boolean crc32;
  private final CompressionAlgorithm compressionAlgorithm;
  private final Map<String, String> requestHeaders;
  private final Long createInteractionId;
  private final Long interactionId;

  protected RpcOptions(
      ProtocolId protocolId,
      Integer seqId,
      Integer clientTimeoutMs,
      Integer queueTimeoutMs,
      RpcPriority priority,
      Map<String, String> otherMetadata,
      boolean crc32,
      CompressionAlgorithm compressionAlgorithm,
      Map<String, String> requestHeaders,
      Long createInteractionId,
      Long interactionId) {
    this.protocolId = protocolId;
    this.seqId = seqId;
    this.clientTimeoutMs = clientTimeoutMs;
    this.queueTimeoutMs = queueTimeoutMs;
    this.priority = priority;
    this.otherMetadata = otherMetadata;
    this.crc32 = crc32;
    this.compressionAlgorithm = compressionAlgorithm;
    this.requestHeaders = requestHeaders;
    this.createInteractionId = createInteractionId;
    this.interactionId = interactionId;
  }

  public static class Builder {
    private ProtocolId protocolId;
    private Integer seqId;
    private Integer clientTimeoutMs;
    private Integer queueTimeoutMs;
    private RpcPriority priority;
    private Map<String, String> otherMetadata;
    private boolean crc32;
    private CompressionAlgorithm compressionAlgorithm;
    private Map<String, String> requestHeaders = new HashMap<>();
    private Long createInteractionId = null;
    private Long interactionId = null;

    public Builder() {}

    public Builder(RpcOptions rpcOptions) {
      this.protocolId = rpcOptions.getProtocolId();
      this.seqId = rpcOptions.getSeqId();
      this.clientTimeoutMs = rpcOptions.getClientTimeoutMs();
      this.queueTimeoutMs = rpcOptions.getQueueTimeoutMs();
      this.priority = rpcOptions.getPriority();
      this.otherMetadata = rpcOptions.getOtherMetadata();
      this.crc32 = rpcOptions.isCrc32();
      this.compressionAlgorithm = rpcOptions.getCompressionAlgorithm();
      this.requestHeaders = rpcOptions.getRequestHeaders();
    }

    public ProtocolId getProtocolId() {
      return protocolId;
    }

    public Builder setProtocolId(ProtocolId protocolId) {
      this.protocolId = protocolId;
      return this;
    }

    public Integer getSeqId() {
      return seqId;
    }

    public Builder setSeqId(Integer seqId) {
      this.seqId = seqId;
      return this;
    }

    public Integer getClientTimeoutMs() {
      return clientTimeoutMs;
    }

    public Builder setClientTimeoutMs(Integer clientTimeoutMs) {
      this.clientTimeoutMs = clientTimeoutMs;
      return this;
    }

    public Integer getQueueTimeoutMs() {
      return queueTimeoutMs;
    }

    public Builder setQueueTimeoutMs(Integer queueTimeoutMs) {
      this.queueTimeoutMs = queueTimeoutMs;
      return this;
    }

    public RpcPriority getPriority() {
      return priority;
    }

    public Builder setPriority(RpcPriority priority) {
      this.priority = priority;
      return this;
    }

    public Map<String, String> getOtherMetadata() {
      return otherMetadata;
    }

    public Builder setOtherMetadata(Map<String, String> otherMetadata) {
      this.otherMetadata = otherMetadata;
      return this;
    }

    public boolean isCrc32() {
      return crc32;
    }

    public Builder setCrc32(boolean crc32) {
      this.crc32 = crc32;
      return this;
    }

    public CompressionAlgorithm getCompressionAlgorithm() {
      return compressionAlgorithm;
    }

    public Builder setCompressionAlgorithm(CompressionAlgorithm compressionAlgorithm) {
      this.compressionAlgorithm = compressionAlgorithm;
      return this;
    }

    public Map<String, String> getRequestHeaders() {
      return requestHeaders;
    }

    public Builder setRequestHeaders(@Nonnull Map<String, String> requestHeaders) {
      Preconditions.checkNotNull(requestHeaders);
      this.requestHeaders = requestHeaders;
      return this;
    }

    public Long getCreateInteractionId() {
      return createInteractionId;
    }

    public Builder setCreateInteractionId(Long createInteractionId) {
      this.createInteractionId = createInteractionId;
      return this;
    }

    public Long getInteractionId() {
      return interactionId;
    }

    public Builder setInteractionId(Long interactionId) {
      this.interactionId = interactionId;
      return this;
    }

    public RpcOptions build() {
      return new RpcOptions(
          protocolId,
          seqId,
          clientTimeoutMs,
          queueTimeoutMs,
          priority,
          otherMetadata,
          crc32,
          compressionAlgorithm,
          requestHeaders,
          createInteractionId,
          interactionId);
    }
  }

  public ProtocolId getProtocolId() {
    return protocolId;
  }

  public Integer getSeqId() {
    return seqId;
  }

  public Integer getClientTimeoutMs() {
    return clientTimeoutMs;
  }

  public Integer getQueueTimeoutMs() {
    return queueTimeoutMs;
  }

  public RpcPriority getPriority() {
    return priority;
  }

  public Map<String, String> getOtherMetadata() {
    return otherMetadata;
  }

  public boolean isCrc32() {
    return crc32;
  }

  public CompressionAlgorithm getCompressionAlgorithm() {
    return compressionAlgorithm;
  }

  public Map<String, String> getRequestHeaders() {
    return requestHeaders;
  }

  public Long getCreateInteractionId() {
    return createInteractionId;
  }

  public Long getInteractionId() {
    return interactionId;
  }

  @Override
  public String toString() {
    StringBuilder builder = new StringBuilder("RpcOptions(");
    builder.append("protocolId=").append(getProtocolId());
    builder.append(", seqId=").append(getSeqId());
    builder.append(", clientTimeoutMs=").append(getClientTimeoutMs());
    builder.append(", priority=").append(getPriority());
    builder.append(", otherMetadata=").append(getOtherMetadata());
    builder.append(", crc32?=").append(isCrc32());
    builder.append(", compressionAlgo=").append(getCompressionAlgorithm());
    builder.append(", requestHeaders=").append(getRequestHeaders());
    builder.append(", createInteractionId=").append(getCreateInteractionId());
    builder.append(", interactionId=").append(getInteractionId());
    builder.append(')');
    return builder.toString();
  }
}
