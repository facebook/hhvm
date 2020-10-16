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

enum class F: ExBox extends E {
  C<Box<num>>(new Box(3.14));
}

abstract class Base {
  abstract const type TEnum as E;

  public function get<T>(HH\Elt<this::TEnum, Box<T>> $param): T {
    return $param->unwrap()->data;
  }

  public static function dictGen(): dict<string, ExBox> {
    $dict = dict[];
    $ts = type_structure(static::class, 'TEnum');
    $cls = $ts['classname'];
    foreach ($cls::getValues() as $key => $elt) {
      $dict[$key] = $elt->unwrap();
    }
    return $dict;
  }
}

class Ext extends Base {
  const type TEnum = E; // [0]
}

function test_constraint(): void {
  $ext = new Ext();
  echo $ext->get(F::C); // F vs E, because of [0].
  echo "\n";
}
