<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test_invariant(): void {
  invariant('false', 'This will never crash');
}

function test_not_invariant(): void {
  invariant(!'false', 'This will always crash');
}

function test_if(): void {
  if ('false') {
    printf('This will always happen');
  }
}

function test_if_not(): void {
  if (!'false') {
    printf('This will never happen');
  }
}

function test_double_not(): void {
  if (!!'false') {
    printf('This will always happen');
  }
}

// negative examples below

function test_if_var(): void {
  $false = false;
  if ($false) {
    invariant_violation('this should never happen');
  }
}

function test_invariant_violation(): void {
  invariant_violation('This shouldn\'t cause a lint error');
}

function test_function_on_string(): void {
  if (is_foo('foo')) {
    invariant_violation('');
  }
}
