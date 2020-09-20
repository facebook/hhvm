<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I1 {
  public function get<T>(T $in): void;
}

final class C1<T as string> implements I1 {
  final public function get(string $in): void {}
}

abstract final class A1 {
  final public static function bug(I1 $o, int $i): void {
    $o->get($i);
  }
}
