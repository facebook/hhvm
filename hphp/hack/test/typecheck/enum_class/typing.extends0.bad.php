<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// User code
function expect_int(int $_): void {}
function expect_string(string $_): void {}
function expect_num(num $_): void {}

interface ExBox {}

class Box<T> implements ExBox {
  public function __construct(public T $data)[] {}
}

class IBox extends Box<int> {
  public function add(int $x)[write_props]: void {
    $this->data = $this->data + $x;
  }
}

abstract final class Helper {
  public static function ibox()[]: IBox {
    return new IBox(42);
  }
}

// Enum Class user code
enum class E: ExBox {
   Box<string> A = new Box('bli');
   IBox B = Helper::ibox();
   Box<int> B2 = new Box(42);
}

function e<T>(HH\MemberOf<E, Box<T>> $param): T {
  return $param->data;
}

enum class F: ExBox extends E {
   Box<num> C = new Box(3.14);
}

function testit3(): void {
  // errors correctly (need to improve error message, super is confusing)
  expect_num(e(F::C));
}
