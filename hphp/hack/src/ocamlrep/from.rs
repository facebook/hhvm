// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Helpers for implementing `OcamlRep::from_ocamlrep`.

use std::convert::TryInto;

use crate::{Block, FromError, OcamlRep, Value};

pub fn expect_int(value: Value<'_>) -> Result<isize, FromError> {
    match value.as_int() {
        Some(value) => Ok(value),
        None => Err(FromError::ExpectedImmediate(value.to_bits())),
    }
}

pub fn expect_nullary_variant(value: Value<'_>, max: usize) -> Result<isize, FromError> {
    let value = expect_int(value)?;
    let max_as_isize: isize = max.try_into().unwrap();
    if 0 <= value && value <= max_as_isize {
        Ok(value)
    } else {
        Err(FromError::NullaryVariantTagOutOfRange { max, actual: value })
    }
}

pub fn expect_block<'a>(value: Value<'a>) -> Result<Block<'a>, FromError> {
    match value.as_block() {
        Some(block) => Ok(block),
        None => Err(FromError::ExpectedBlock(value.as_int().unwrap())),
    }
}

pub fn expect_block_size(block: Block<'_>, size: usize) -> Result<(), FromError> {
    if block.size() != size {
        return Err(FromError::WrongBlockSize {
            expected: size,
            actual: block.size(),
        });
    }
    Ok(())
}

pub fn expect_block_tag(block: Block<'_>, tag: u8) -> Result<(), FromError> {
    if block.tag() != tag {
        return Err(FromError::ExpectedBlockTag {
            expected: tag,
            actual: block.tag(),
        });
    }
    Ok(())
}

pub fn expect_tuple<'a>(value: Value<'a>, size: usize) -> Result<Block<'a>, FromError> {
    let block = expect_block(value)?;
    expect_block_size(block, size)?;
    if block.tag() != 0 {
        return Err(FromError::ExpectedZeroTag(block.tag()));
    }
    Ok(block)
}

pub fn field<T: OcamlRep>(block: Block<'_>, field: usize) -> Result<T, FromError> {
    T::from_ocamlrep(block[field]).map_err(|e| FromError::ErrorInField(field, Box::new(e)))
}
