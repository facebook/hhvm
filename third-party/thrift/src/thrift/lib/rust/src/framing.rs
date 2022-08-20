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

use std::io::Cursor;

use bytes::Bytes;
use bytes::BytesMut;

use crate::bufext::BufExt;
use crate::bufext::BufMutExt;

/// Helper type alias to get encoding buffer type
pub type FramingEncoded<F> = <F as Framing>::EncBuf;

/// Helper type alias to get the type of the finalized encoded buffer
pub type FramingEncodedFinal<F> = <<F as Framing>::EncBuf as BufMutExt>::Final;

/// Helper type alias to get the buffer to use as input to decoding
pub type FramingDecoded<F> = <F as Framing>::DecBuf;

/// Trait describing the in-memory frames the transport uses for Protocol messages.
pub trait Framing {
    /// Buffer type we encode into
    type EncBuf: BufMutExt + Send + 'static;

    /// Buffer type we decode from
    type DecBuf: BufExt + Send + 'static;

    /// Allocate a new encoding buffer with a given capacity
    /// FIXME: need &self?
    fn enc_with_capacity(cap: usize) -> Self::EncBuf;
}

impl Framing for Bytes {
    type EncBuf = BytesMut;
    type DecBuf = Cursor<Bytes>;

    fn enc_with_capacity(cap: usize) -> Self::EncBuf {
        BytesMut::with_capacity(cap)
    }
}
