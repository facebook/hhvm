<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Evil<T> {}

<<__SupportDynamicType>>
class C {
  public static function foo(Evil<int> $v) : Evil<int> {
    return $v;
  }

  public function bar(Evil<int> $v) : Evil<int> {
    return self::foo($v);
  }
}
