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

function testit(): void {
  expect_string(E::A->data);
  expect_int(E::B->data);

  expect_string(e(E::A));
  expect_int(e(E::B));
}

function generic<TEnum as E, T>(HH\MemberOf<TEnum, Box<T>> $param): T {
  return $param->data;
}

function generic2<TEnum as E, TBox as IBox>(
  HH\MemberOf<TEnum, TBox> $param,
): int {
  $param->add(42);
  return $param->data;
}

function testit2(): void {
  expect_string(generic(E::A));
  expect_int(generic(E::B));

  expect_int(generic2(E::B));
}

function iterate(): void {
  foreach (E::getValues() as $key => $elt) {
    echo "$key = ";
    $box = $elt as Box<_>;
    echo (string) $box->data;
    echo "\n";
  }
}

/* Listing some valid base types */
interface I0<T> {}

// closed interface
enum class EI0: I0<IBox> {}

class C0<T> {}

// closed class
enum class EC0: C0<IBox> {}


// nonnull and mixed
enum class ENN: nonnull {
   int A = 42;
   string B = "zuck";
   Box<int> C = new IBox(42);
}

enum class EM: mixed {
   int A = 42;
   ?string B = null;
   Box<int> C = new IBox(42);
}

// tuples
enum class ETuple: (IBox, IBox) {}


// primitive types
enum class Ints: int {}

enum class Strings: string {}
