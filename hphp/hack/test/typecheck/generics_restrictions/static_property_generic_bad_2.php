<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

// trait itself is ok
trait Tr<T> {
  public static ?T $fld = null;
  public static vec<T> $vec_fld = vec[];
}

// This is ok
class WithInt {
  use Tr<int>;
}

class C<T> {
}

// This is not ok
class WithAny<T> {
  use Tr<C<T>>;
}
