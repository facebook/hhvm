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

package com.facebook.thrift.legacy.exceptions;

import static com.google.common.base.Preconditions.checkArgument;
import static io.airlift.units.DataSize.succinctBytes;
import static java.lang.String.format;
import static java.util.Objects.requireNonNull;

import com.facebook.thrift.legacy.codec.FrameInfo;
import io.netty.handler.codec.DecoderException;
import java.util.Optional;

public class FrameTooLargeException extends DecoderException {
  private final Optional<FrameInfo> frameInfo;
  private final long frameSizeInBytes;
  private final int maxFrameSizeInBytes;

  public FrameTooLargeException(
      Optional<FrameInfo> frameInfo, long frameSizeInBytes, int maxFrameSizeInBytes) {
    this.frameInfo = requireNonNull(frameInfo, "sequenceId is null");
    checkArgument(frameSizeInBytes >= 0, "frameSizeInBytes cannot be negative");
    this.frameSizeInBytes = frameSizeInBytes;
    checkArgument(maxFrameSizeInBytes >= 0, "maxFrameSizeInBytes cannot be negative");
    this.maxFrameSizeInBytes = maxFrameSizeInBytes;
  }

  public Optional<FrameInfo> getFrameInfo() {
    return frameInfo;
  }

  @Override
  public String getMessage() {
    return format(
        "Frame size %s exceeded max size %s: %s",
        succinctBytes(frameSizeInBytes), succinctBytes(maxFrameSizeInBytes), frameInfo);
  }
}
