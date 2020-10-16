<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file: __EnableUnstableFeatures('enum_class')>>

interface ExBox {}

class Box<T> implements ExBox {
  public function __construct(public T $data) {}
}

class IBox extends Box<int> {
  public function add(int $x): void {
    $this->data = $this->data + $x;
  }
}

abstract final class Helper {
  public static function ibox(): IBox {
    return new IBox(42);
  }
}

enum class E: ExBox {
  A<Box<string>>(new Box('bli'));
  B<IBox>(Helper::ibox());
  C<Box<int>>(new Box(42));
}
