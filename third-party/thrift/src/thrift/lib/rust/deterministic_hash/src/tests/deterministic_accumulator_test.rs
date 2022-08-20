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

use fbthrift::TType;
use fbthrift_deterministic_hash::DeterministicAccumulator;
use fbthrift_deterministic_hash::DeterministicAccumulatorError;
use fbthrift_deterministic_hash::Sha256Hasher;
// use teststructs::TestSkipV2;

#[test]
fn deterministic_accumulator_combine_one_value() {
    let mut da = DeterministicAccumulator::new(Sha256Hasher::default);
    da.combine(5i8);
    assert!(da.get_result().is_ok(), "no error expected");
}

#[test]
fn deterministic_accumulator_test_unordered() {
    let mut da1 = DeterministicAccumulator::new(Sha256Hasher::default);
    da1.begin_unordered();
    da1.combine(1i8);
    da1.combine(2i8);
    da1.combine(3i8);
    da1.combine(1i8);
    da1.end_unordered();

    let mut da2 = DeterministicAccumulator::new(Sha256Hasher::default);
    da2.begin_unordered();
    da2.combine(3i8);
    da2.combine(1i8);
    da2.combine(2i8);
    da2.combine(1i8);
    da2.end_unordered();

    let result1 = da1.get_result().expect("no error expected");

    let result2 = da2.get_result().expect("no error expected");

    assert_eq!(result1, result2);
}

#[test]
fn deterministic_accumulator_test_ordered() {
    let mut da1 = DeterministicAccumulator::new(Sha256Hasher::default);
    da1.begin_ordered();
    da1.combine(1i8);
    da1.combine(2i8);
    da1.combine(3i8);
    da1.combine(1i8);
    da1.end_ordered();

    let mut da2 = DeterministicAccumulator::new(Sha256Hasher::default);
    da2.begin_ordered();
    da2.combine(3i8);
    da2.combine(1i8);
    da2.combine(2i8);
    da2.combine(1i8);
    da2.end_ordered();

    let result1 = da1.get_result().expect("no error expected");

    let result2 = da2.get_result().expect("no error expected");

    assert_ne!(result1, result2);
}

#[test]
fn deterministic_accumulator_test_nested() {
    //Examples:
    // struct MyData {
    //  1: list<i64> f1 = [1, 2, 3];
    //  2: set<i64>  f2 = {4, 5, 6};
    // }
    let mut acc = DeterministicAccumulator::new(Sha256Hasher::default);
    acc.begin_unordered(); // struct data begin
    acc.begin_ordered(); // field  f1   begin
    acc.combine(TType::List as i8); // field  f1   type
    acc.combine(1i8); // field  f1   id
    acc.begin_ordered(); // list   f1   begin
    acc.combine(TType::I64 as i8); // list   f1   type
    acc.combine(3i8); // list   f1   size
    acc.begin_ordered(); // list   f1   data begin
    acc.combine(1i8); // f1[0]
    acc.combine(2i8); // f1[1]
    acc.combine(3i8); // f1[2]
    acc.end_ordered(); // list   f1   data end
    acc.end_ordered(); // list   f1   end
    acc.end_ordered(); // field  f1   end
    acc.begin_ordered(); // field  f2   begin
    acc.combine(TType::Set as i8); // field  f2   type
    acc.combine(2i8); // field  f2   id
    acc.begin_ordered(); // set    f2   begin
    acc.combine(TType::String as i8); // set    f2   type
    acc.combine(3i8); // set    f2   size
    acc.begin_unordered(); // set    f2   data begin
    acc.combine(4i8); // f2[0]
    acc.combine(5i8); // f2[1]
    acc.combine(6i8); // f2[2]
    acc.end_unordered(); // set    f2   data end
    acc.end_ordered(); // set    f2   end
    acc.end_ordered(); // field  f2   end
    acc.end_unordered(); // struct data end

    assert_eq!(
        acc.get_result().expect("no error expected"),
        [
            133, 15, 18, 181, 19, 1, 94, 129, 18, 56, 65, 203, 169, 152, 120, 251, 250, 215, 192,
            101, 19, 89, 174, 115, 109, 217, 229, 172, 156, 148, 0, 57
        ],
    );
}

#[test]
fn deterministic_accumulator_end_ordered_on_empty() {
    let mut da = DeterministicAccumulator::new(Sha256Hasher::default);
    da.end_ordered();
    assert_eq!(
        da.get_result()
            .err()
            .expect("error is expected")
            .downcast::<DeterministicAccumulatorError>()
            .expect("DeterministicAccumulatorError is expected"),
        DeterministicAccumulatorError::EndOrderedOnEmpty
    );
}

#[test]
fn deterministic_accumulator_end_unordered_on_empty() {
    let mut da = DeterministicAccumulator::new(Sha256Hasher::default);
    da.end_unordered();
    assert_eq!(
        da.get_result()
            .err()
            .expect("error is expected")
            .downcast::<DeterministicAccumulatorError>()
            .expect("DeterministicAccumulatorError is expected"),
        DeterministicAccumulatorError::EndUnorderedOnEmpty
    );
}

#[test]
fn deterministic_accumulator_end_unordered_on_ordered() {
    let mut da = DeterministicAccumulator::new(Sha256Hasher::default);
    da.begin_ordered();
    da.end_unordered();
    assert_eq!(
        da.get_result()
            .err()
            .expect("error is expected")
            .downcast::<DeterministicAccumulatorError>()
            .expect("DeterministicAccumulatorError is expected"),
        DeterministicAccumulatorError::EndUnorderedOnOrdered
    );
}

#[test]
fn deterministic_accumulator_end_ordered_on_unordered() {
    let mut da = DeterministicAccumulator::new(Sha256Hasher::default);
    da.begin_unordered();
    da.end_ordered();
    assert_eq!(
        da.get_result()
            .err()
            .expect("error is expected")
            .downcast::<DeterministicAccumulatorError>()
            .expect("DeterministicAccumulatorError is expected"),
        DeterministicAccumulatorError::EndOrderedOnUnordered
    );
}
