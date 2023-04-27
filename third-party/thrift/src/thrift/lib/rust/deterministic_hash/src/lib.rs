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

pub mod deterministic_accumulator;
pub mod deterministic_protocol;
pub mod hasher;
use anyhow::Result;
use fbthrift::protocol::ProtocolWriter;
use fbthrift::Serialize;

pub use crate::deterministic_accumulator::DeterministicAccumulator;
pub use crate::deterministic_accumulator::DeterministicAccumulatorError;
use crate::deterministic_protocol::DeterministicProtocolSerializer;
pub use crate::hasher::Hasher;
pub use crate::hasher::Sha256Hasher;
pub fn deterministic_hash<
    H: Hasher + Default,
    F: Fn() -> H,
    S: Serialize<DeterministicProtocolSerializer<H, F>>,
>(
    data: &S,
    hasher_generator: F,
) -> Result<H::Output> {
    let mut protocol_writer = DeterministicProtocolSerializer::new(hasher_generator);
    data.write(&mut protocol_writer);
    protocol_writer.finish()
}
