<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I { }
interface J { }

function test<T as (function(): I) as (function(): J)>(T $f): I {
  $x = $f();
  return $x;
}

function test2<T as (function(): I) as (function(): J)>(T $f): I {
  // Test bidirectional type checking: we should not attempt to use I
  // when checking $f()
  return $f();
}

function test3<T as (function(): I) as (function(): J)>(T $f): J {
  $x = $f();
  return $x;
}

function test4<T as (function(): I) as (function(): J)>(T $f): J {
  // Test bidirectional type checking: we should not attempt to use J
  // when checking $f()
  return $f();
}
