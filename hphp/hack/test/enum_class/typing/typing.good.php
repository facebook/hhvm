<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file: __EnableUnstableFeatures('enum_class')>>

// User code
function expect_int(int $_): void {}
function expect_string(string $_): void {}
function expect_num(num $_): void {}

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

// Enum Class user code
enum class E: ExBox {
  A<Box<string>>(new Box('bli'));
  B<IBox>(Helper::ibox());
  B2<Box<int>>(new Box(42));
}

function e<T>(HH\Elt<E, Box<T>> $param): T {
  return $param->unwrap()->data;
}

function testit(): void {
  expect_string(E::A->unwrap()->data);
  expect_int(E::B->unwrap()->data);

  expect_string(e(E::A));
  expect_int(e(E::B));
}

function generic<TEnum as E, T>(HH\Elt<TEnum, Box<T>> $param): T {
  return $param->unwrap()->data;
}

function generic2<TEnum as E, TBox as IBox>(HH\Elt<TEnum, TBox> $param): int {
  $param->unwrap()->add(42);
  return $param->unwrap()->data;
}

function testit2(): void {
  expect_string(generic(E::A));
  expect_int(generic(E::B));

  expect_int(generic2(E::B));
}

function iterate(): void {
  foreach (E::getValues() as $key => $elt) {
    echo "$key = ";
    $exbox = $elt->unwrap();
    $box = $exbox as Box<_>;
    echo $box->data;
    echo "\n";
  }
}
