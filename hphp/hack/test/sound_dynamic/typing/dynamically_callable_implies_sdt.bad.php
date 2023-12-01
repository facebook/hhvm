<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

class C { }

<<__DynamicallyCallable>>
function foo():C {
  // Expect error because C is not dynamic
  return new C();
}
