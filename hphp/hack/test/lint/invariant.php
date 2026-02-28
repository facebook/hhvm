<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test_invariant_fail1(): void {
  invariant(true, 'This should cause lint error');
}

function test_invariant_fail2(): void {
  invariant(!!true, 'This should cause lint error');
}

function test_invariant_fail3(): void {
  invariant(!true, 'This should cause lint error');
}

function test_invariant_fail4(): void {
  // invariant(false) is a special case
  // it is converted directly into the call to invariant_violation
  invariant(false, 'This should not cause lint error');
}

function test_invariant_ok(bool $cond): void {
  invariant($cond, 'This should not cause lint error');
}
