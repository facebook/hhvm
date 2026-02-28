<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

namespace A {
  class T {}

  class C<T> {} // bad
}

namespace {
  class C<T> {} // ok
}
