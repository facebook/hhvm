<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

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
  Box<string> A = new Box('bli');
  IBox B = Helper::ibox();
  Box<int> C = new Box(42);
}

enum class F: ExBox extends E {
  Box<int> D = new Box(1664);
}
