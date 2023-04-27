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

pub trait Hasher {
    type Output: Ord;
    fn finalize(self) -> Self::Output;
    fn combine_bool(&mut self, value: bool);
    fn combine_i8(&mut self, value: i8);
    fn combine_i16(&mut self, value: i16);
    fn combine_i32(&mut self, value: i32);
    fn combine_i64(&mut self, value: i64);
    fn combine_f32(&mut self, value: f32);
    fn combine_f64(&mut self, value: f64);
    fn combine_bytes(&mut self, value: &[u8]);
    // fn combine(&mut self,const folly::IOBuf&mut );
    fn combine_hasher(&mut self, value: &Self::Output);
}

use ring::digest::Context;
use ring::digest::SHA256;

pub struct Sha256Hasher {
    context: Context,
}

impl Default for Sha256Hasher {
    fn default() -> Self {
        Sha256Hasher {
            context: Context::new(&SHA256),
        }
    }
}

impl Hasher for Sha256Hasher {
    type Output = [u8; 32];
    fn finalize(self) -> Self::Output {
        self.context
            .finish()
            .as_ref()
            .try_into()
            .expect("Sha256 hash should be 32 bytes!")
    }
    fn combine_bool(&mut self, value: bool) {
        if value {
            self.context.update(&[1u8; 1])
        } else {
            self.context.update(&[0u8; 1])
        }
    }
    fn combine_i8(&mut self, value: i8) {
        self.context.update(&value.to_le_bytes())
    }
    fn combine_i16(&mut self, value: i16) {
        self.context.update(&value.to_le_bytes())
    }
    fn combine_i32(&mut self, value: i32) {
        self.context.update(&value.to_le_bytes())
    }
    fn combine_i64(&mut self, value: i64) {
        self.context.update(&value.to_le_bytes())
    }
    fn combine_f32(&mut self, value: f32) {
        self.context.update(&value.to_le_bytes())
    }
    fn combine_f64(&mut self, value: f64) {
        self.context.update(&value.to_le_bytes())
    }
    fn combine_bytes(&mut self, value: &[u8]) {
        self.context.update(value)
    }
    // fn combine(self: &mutself,Sha256Hasherconst folly::IOBuf&mut);
    fn combine_hasher(&mut self, value: &Self::Output) {
        self.context.update(value)
    }
}
