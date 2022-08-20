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

use anyhow::Result;
use thiserror::Error;

pub use crate::hasher::Hasher;

#[derive(Error, Debug, PartialEq)]
pub enum DeterministicAccumulatorError {
    #[error("nothing has been written to the accumulator")]
    NothingWritten,
    #[error("end_ordered called on empty context")]
    EndOrderedOnEmpty,
    #[error("end_unordered called on empty context")]
    EndUnorderedOnEmpty,
    #[error("end_ordered called on unordered context")]
    EndOrderedOnUnordered,
    #[error("end_unordered called on ordered context")]
    EndUnorderedOnOrdered,
}

struct OrderedContext<T: Hasher> {
    pub count: u64,
    pub hasher: T,
}

enum Context<T: Hasher> {
    Ordered(OrderedContext<T>),
    Unordered(Vec<T::Output>),
}

pub struct DeterministicAccumulator<T: Hasher, F: Fn() -> T> {
    generator: F,
    result: Option<T::Output>,
    error: Option<DeterministicAccumulatorError>,
    context: Vec<Context<T>>,
}

impl<T: Hasher, F: Fn() -> T> DeterministicAccumulator<T, F> {
    pub fn new(generator: F) -> DeterministicAccumulator<T, F> {
        DeterministicAccumulator {
            generator,
            result: None,
            error: None,
            context: Vec::new(),
        }
    }

    pub fn get_result(self) -> Result<T::Output> {
        if let Some(error) = self.error {
            return Result::Err(error.into());
        }
        match self.result {
            None => Err(DeterministicAccumulatorError::NothingWritten.into()),
            Some(result) => Ok(result),
        }
    }

    pub fn begin_unordered(&mut self) {
        self.context.push(Context::Unordered(Vec::new()));
    }
    pub fn begin_ordered(&mut self) {
        let mut current_context = self.context.last_mut();

        match current_context {
            None => {
                self.context.push(Context::Ordered(OrderedContext {
                    count: 1,
                    hasher: (self.generator)(),
                }));
            }
            Some(ref mut context) => match context {
                Context::Ordered(ref mut ordered_context) => {
                    ordered_context.count += 1;
                }
                Context::Unordered(_) => {
                    self.context.push(Context::Ordered(OrderedContext {
                        count: 1,
                        hasher: (self.generator)(),
                    }));
                }
            },
        }
    }

    pub fn end_unordered(&mut self) {
        match self.context.pop() {
            Some(current_context) => {
                if let Context::Unordered(mut unordered_context) = current_context {
                    unordered_context.sort();
                    let mut hasher = (self.generator)();
                    for result in unordered_context {
                        hasher.combine_hasher(&result);
                    }
                    self.exit_context(hasher.finalize())
                } else {
                    self.set_error(DeterministicAccumulatorError::EndUnorderedOnOrdered);
                }
            }
            None => self.set_error(DeterministicAccumulatorError::EndUnorderedOnEmpty),
        }
    }

    pub fn end_ordered(&mut self) {
        match self.context.pop() {
            Some(current_context) => {
                if let Context::Ordered(mut ordered_context) = current_context {
                    ordered_context.count -= 1;
                    if ordered_context.count == 0 {
                        let hasher = ordered_context.hasher;
                        self.exit_context(hasher.finalize())
                    } else {
                        self.context.push(Context::Ordered(ordered_context));
                    }
                } else {
                    self.set_error(DeterministicAccumulatorError::EndOrderedOnUnordered);
                }
            }
            None => self.set_error(DeterministicAccumulatorError::EndOrderedOnEmpty),
        }
    }

    pub fn exit_context(&mut self, result: T::Output) {
        let mut current_context = self.context.last_mut();

        match current_context {
            None => {
                self.result = Some(result);
            }
            Some(ref mut context) => match context {
                Context::Ordered(ref mut ordered_context) => {
                    ordered_context.hasher.combine_hasher(&result);
                }
                Context::Unordered(ref mut unordered_context) => {
                    unordered_context.push(result);
                }
            },
        }
    }
    pub fn combine<V: Hashable<T>>(&mut self, value: V) {
        let mut current_context = self.context.last_mut();
        match current_context {
            None => {
                let mut hasher = (self.generator)();
                value.add_to_hasher(&mut hasher);
                self.result = Some(hasher.finalize());
            }
            Some(ref mut context) => match context {
                Context::Ordered(ref mut ordered_context) => {
                    value.add_to_hasher(&mut ordered_context.hasher);
                }
                Context::Unordered(ref mut unordered_context) => {
                    let mut hasher = (self.generator)();
                    value.add_to_hasher(&mut hasher);
                    unordered_context.push(hasher.finalize());
                }
            },
        }
    }

    fn set_error(&mut self, error: DeterministicAccumulatorError) {
        match self.error {
            None => self.error = Some(error),
            Some(_) => {}
        }
    }
}

pub trait Hashable<H: Hasher> {
    fn add_to_hasher(&self, hasher: &mut H);
}

impl<H: Hasher> Hashable<H> for bool {
    fn add_to_hasher(&self, hasher: &mut H) {
        hasher.combine_bool(*self);
    }
}

impl<H: Hasher> Hashable<H> for i8 {
    fn add_to_hasher(&self, hasher: &mut H) {
        hasher.combine_i8(*self);
    }
}

impl<H: Hasher> Hashable<H> for i16 {
    fn add_to_hasher(&self, hasher: &mut H) {
        hasher.combine_i16(*self);
    }
}

impl<H: Hasher> Hashable<H> for i32 {
    fn add_to_hasher(&self, hasher: &mut H) {
        hasher.combine_i32(*self);
    }
}

impl<H: Hasher> Hashable<H> for i64 {
    fn add_to_hasher(&self, hasher: &mut H) {
        hasher.combine_i64(*self);
    }
}

impl<H: Hasher> Hashable<H> for f32 {
    fn add_to_hasher(&self, hasher: &mut H) {
        hasher.combine_f32(*self);
    }
}

impl<H: Hasher> Hashable<H> for f64 {
    fn add_to_hasher(&self, hasher: &mut H) {
        hasher.combine_f64(*self);
    }
}

impl<H: Hasher> Hashable<H> for &[u8] {
    fn add_to_hasher(&self, hasher: &mut H) {
        hasher.combine_bytes(*self);
    }
}
