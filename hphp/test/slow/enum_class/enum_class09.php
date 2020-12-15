<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file: __EnableUnstableFeatures('enum_class')>>

interface ExBox {}

class Box<T> implements ExBox {
  public function __construct(public T $data) {}
}

class IBox extends Box<int> {
  public function add(int $x) : void { $this->data = $this->data + $x; }
}

enum class E : ExBox {
  A<Box<string>>(new Box('zuck'));
  B<IBox>(new IBox(42));
}

enum class F : ExBox extends E {
  C<Box<num>>(new Box(3.14));
}

<<__EntryPoint>>
function main(): void {
  echo E::A->data()->data;
  echo "\n";

  echo "Testing E's getValues()\n";
  foreach (E::getValues() as $key => $value) {
    echo "$key = ";
    $box = $value->data() as Box<_>;
    echo $box->data;
    echo "\n";
  }

  echo "Testing F\n";
  echo F::A->data()->data;
  echo "\n";
  echo F::C->data()->data;
  echo "\n";
  echo "Testing F's getValues()\n";
  foreach (F::getValues() as $key => $value) {
    echo "$key = ";
    $box = $value->data() as Box<_>;
    echo $box->data;
    echo "\n";
  }

  echo "Testing ->name()\n";
  echo "F::C->name() = ";
  echo F::C->name();
  echo "\n";
}
