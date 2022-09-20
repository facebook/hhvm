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

package com.facebook.thrift;

import java.nio.ByteBuffer;

public class VarIntUtils {
  public static int readVarInt32(ByteBuffer src) {
    int tmp;
    if ((tmp = src.get()) >= 0) {
      return tmp;
    }
    int result = tmp & 0x7f;
    if ((tmp = src.get()) >= 0) {
      result |= tmp << 7;
    } else {
      result |= (tmp & 0x7f) << 7;
      if ((tmp = src.get()) >= 0) {
        result |= tmp << 14;
      } else {
        result |= (tmp & 0x7f) << 14;
        if ((tmp = src.get()) >= 0) {
          result |= tmp << 21;
        } else {
          result |= (tmp & 0x7f) << 21;
          result |= (tmp = src.get()) << 28;
          while (tmp < 0) {
            // We get into this loop only in the case of overflow.
            // By doing this, we can call getVarInt() instead of
            // getVarLong() when we only need an int.
            tmp = src.get();
          }
        }
      }
    }
    return result;
  }

  public static void writeVarInt32(final int src, ByteBuffer target) {
    if (src > 0x0FFFFFFF || src < 0) {
      target.put((byte) (0x80 | ((src >>> 28))));
    }
    if (src > 0x1FFFFF || src < 0) {
      target.put((byte) (0x80 | ((src >>> 21) & 0x7F)));
    }
    if (src > 0x3FFF || src < 0) {
      target.put((byte) (0x80 | ((src >>> 14) & 0x7F)));
    }
    if (src > 0x7F || src < 0) {
      target.put((byte) (0x80 | ((src >>> 7) & 0x7F)));
    }
    target.put((byte) (src & 0x7F));
  }

  public static void writeVarInt64(long src, ByteBuffer target) {
    if (src < 0) {
      target.put((byte) 0x81);
    }
    if (src > 0xFFFFFFFFFFFFFFL || src < 0) {
      target.put((byte) (0x80 | ((src >>> 56) & 0x7FL)));
    }
    if (src > 0x1FFFFFFFFFFFFL || src < 0) {
      target.put((byte) (0x80 | ((src >>> 49) & 0x7FL)));
    }
    if (src > 0x3FFFFFFFFFFL || src < 0) {
      target.put((byte) (0x80 | ((src >>> 42) & 0x7FL)));
    }
    if (src > 0x7FFFFFFFFL || src < 0) {
      target.put((byte) (0x80 | ((src >>> 35) & 0x7FL)));
    }
    if (src > 0xFFFFFFFL || src < 0) {
      target.put((byte) (0x80 | ((src >>> 28) & 0x7FL)));
    }
    if (src > 0x1FFFFFL || src < 0) {
      target.put((byte) (0x80 | ((src >>> 21) & 0x7FL)));
    }
    if (src > 0x3FFFL || src < 0) {
      target.put((byte) (0x80 | ((src >>> 14) & 0x7FL)));
    }
    if (src > 0x7FL || src < 0) {
      target.put((byte) (0x80 | ((src >>> 7) & 0x7FL)));
    }

    target.put((byte) (src & 0x7FL));
  }

  public static long readVarInt64(ByteBuffer src) {
    long tmp;
    if ((tmp = src.get()) >= 0) {
      return tmp;
    }
    long result = tmp & 0x7f;
    if ((tmp = src.get()) >= 0) {
      result |= tmp << 7;
    } else {
      result |= (tmp & 0x7f) << 7;
      if ((tmp = src.get()) >= 0) {
        result |= tmp << 14;
      } else {
        result |= (tmp & 0x7f) << 14;
        if ((tmp = src.get()) >= 0) {
          result |= tmp << 21;
        } else {
          result |= (tmp & 0x7f) << 21;
          if ((tmp = src.get()) >= 0) {
            result |= tmp << 28;
          } else {
            result |= (tmp & 0x7f) << 28;
            if ((tmp = src.get()) >= 0) {
              result |= tmp << 35;
            } else {
              result |= (tmp & 0x7f) << 35;
              if ((tmp = src.get()) >= 0) {
                result |= tmp << 42;
              } else {
                result |= (tmp & 0x7f) << 42;
                if ((tmp = src.get()) >= 0) {
                  result |= tmp << 49;
                } else {
                  result |= (tmp & 0x7f) << 49;
                  if ((tmp = src.get()) >= 0) {
                    result |= tmp << 56;
                  } else {
                    result |= (tmp & 0x7f) << 56;
                    result |= ((long) src.get()) << 63;
                  }
                }
              }
            }
          }
        }
      }
    }
    return result;
  }
}
