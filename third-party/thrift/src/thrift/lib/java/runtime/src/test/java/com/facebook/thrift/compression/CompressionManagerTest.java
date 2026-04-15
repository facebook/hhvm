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

package com.facebook.thrift.compression;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatThrownBy;

import com.facebook.thrift.compression.lz4.Lz4Compressor;
import com.facebook.thrift.compression.zlib.ZlibCompressor;
import com.facebook.thrift.compression.zstd.ZstdCompressor;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.Unpooled;
import java.nio.charset.StandardCharsets;
import org.apache.thrift.CompressionAlgorithm;
import org.apache.thrift.TTransform;
import org.junit.jupiter.api.Test;

class CompressionManagerTest {

  private final ByteBufAllocator allocator = ByteBufAllocator.DEFAULT;

  // --- fromTTransform tests ---

  @Test
  void fromTTransformMapsZlib() {
    assertThat(CompressionManager.fromTTransform(TTransform.ZLIB.getValue()))
        .isEqualTo(CompressionAlgorithm.ZLIB);
  }

  @Test
  void fromTTransformMapsZstd() {
    assertThat(CompressionManager.fromTTransform(TTransform.ZSTD.getValue()))
        .isEqualTo(CompressionAlgorithm.ZSTD);
  }

  @Test
  void fromTTransformMapsLz4() {
    assertThat(CompressionManager.fromTTransform(TTransform.LZ4.getValue()))
        .isEqualTo(CompressionAlgorithm.LZ4);
  }

  @Test
  void fromTTransformMapsNone() {
    assertThat(CompressionManager.fromTTransform(TTransform.NONE.getValue()))
        .isEqualTo(CompressionAlgorithm.NONE);
  }

  @Test
  void fromTTransformMapsLessVariants() {
    assertThat(CompressionManager.fromTTransform(TTransform.ZLIB_LESS.getValue()))
        .isEqualTo(CompressionAlgorithm.ZLIB_LESS);
    assertThat(CompressionManager.fromTTransform(TTransform.ZSTD_LESS.getValue()))
        .isEqualTo(CompressionAlgorithm.ZSTD_LESS);
    assertThat(CompressionManager.fromTTransform(TTransform.LZ4_LESS.getValue()))
        .isEqualTo(CompressionAlgorithm.LZ4_LESS);
  }

  @Test
  void fromTTransformMapsMoreVariants() {
    assertThat(CompressionManager.fromTTransform(TTransform.ZLIB_MORE.getValue()))
        .isEqualTo(CompressionAlgorithm.ZLIB_MORE);
    assertThat(CompressionManager.fromTTransform(TTransform.ZSTD_MORE.getValue()))
        .isEqualTo(CompressionAlgorithm.ZSTD_MORE);
    assertThat(CompressionManager.fromTTransform(TTransform.LZ4_MORE.getValue()))
        .isEqualTo(CompressionAlgorithm.LZ4_MORE);
  }

  @Test
  void fromTTransformThrowsOnUnknown() {
    assertThatThrownBy(() -> CompressionManager.fromTTransform(99))
        .isInstanceOf(IllegalArgumentException.class)
        .hasMessageContaining("Unknown TTransform ID: 99");
  }

  @Test
  void fromTTransformThrowsOnDeprecatedHmac() {
    assertThatThrownBy(() -> CompressionManager.fromTTransform(2))
        .isInstanceOf(IllegalArgumentException.class);
  }

  // --- getCompressor tests ---

  @Test
  void getCompressorReturnsZlibForAllZlibVariants() {
    assertThat(CompressionManager.getCompressor(CompressionAlgorithm.ZLIB))
        .isInstanceOf(ZlibCompressor.class);
    assertThat(CompressionManager.getCompressor(CompressionAlgorithm.ZLIB_LESS))
        .isInstanceOf(ZlibCompressor.class);
    assertThat(CompressionManager.getCompressor(CompressionAlgorithm.ZLIB_MORE))
        .isInstanceOf(ZlibCompressor.class);
  }

  @Test
  void getCompressorReturnsZstdForAllZstdVariants() {
    assertThat(CompressionManager.getCompressor(CompressionAlgorithm.ZSTD))
        .isInstanceOf(ZstdCompressor.class);
    assertThat(CompressionManager.getCompressor(CompressionAlgorithm.ZSTD_LESS))
        .isInstanceOf(ZstdCompressor.class);
    assertThat(CompressionManager.getCompressor(CompressionAlgorithm.ZSTD_MORE))
        .isInstanceOf(ZstdCompressor.class);
  }

  @Test
  void getCompressorReturnsLz4ForAllLz4Variants() {
    assertThat(CompressionManager.getCompressor(CompressionAlgorithm.LZ4))
        .isInstanceOf(Lz4Compressor.class);
    assertThat(CompressionManager.getCompressor(CompressionAlgorithm.LZ4_LESS))
        .isInstanceOf(Lz4Compressor.class);
    assertThat(CompressionManager.getCompressor(CompressionAlgorithm.LZ4_MORE))
        .isInstanceOf(Lz4Compressor.class);
  }

  @Test
  void getCompressorThrowsForNone() {
    assertThatThrownBy(() -> CompressionManager.getCompressor(CompressionAlgorithm.NONE))
        .isInstanceOf(IllegalArgumentException.class);
  }

  // --- decompressFromTransform tests ---

  @Test
  void decompressFromTransformNoneReturnsDataUnchanged() {
    ByteBuf data = Unpooled.wrappedBuffer("pass through".getBytes(StandardCharsets.UTF_8));
    ByteBuf result =
        CompressionManager.decompressFromTransform(TTransform.NONE.getValue(), allocator, data);
    assertThat(result).isSameAs(data);
    data.release();
  }

  // --- TTransform ID value tests (verify they match C++ RpcMetadata.thrift) ---

  @Test
  void transformIdValuesMatchCppReference() {
    assertThat(TTransform.NONE.getValue()).isEqualTo(0);
    assertThat(TTransform.ZLIB.getValue()).isEqualTo(1);
    assertThat(TTransform.ZSTD.getValue()).isEqualTo(5);
    assertThat(TTransform.LZ4.getValue()).isEqualTo(6);
    assertThat(TTransform.CUSTOM.getValue()).isEqualTo(7);
    assertThat(TTransform.ZLIB_LESS.getValue()).isEqualTo(8);
    assertThat(TTransform.ZSTD_LESS.getValue()).isEqualTo(9);
    assertThat(TTransform.LZ4_LESS.getValue()).isEqualTo(10);
    assertThat(TTransform.ZLIB_MORE.getValue()).isEqualTo(11);
    assertThat(TTransform.ZSTD_MORE.getValue()).isEqualTo(12);
    assertThat(TTransform.LZ4_MORE.getValue()).isEqualTo(13);
  }
}
