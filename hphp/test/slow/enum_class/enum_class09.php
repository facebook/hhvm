<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface ExBox {}

class Box<T> implements ExBox {
  public function __construct(public T $data) {}
}

class IBox extends Box<int> {
  public function add(int $x) : void { $this->data = $this->data + $x; }
}

enum class E : ExBox {
   Box<string> A = new Box('zuck');
   IBox B = new IBox(42);
}

enum class F : ExBox extends E {
   Box<num> C = new Box(3.14);
}

<<__EntryPoint>>
function main(): void {
  echo E::A->data;
  echo "\n";

  echo "Testing E's getValues()\n";
  foreach (E::getValues() as $key => $value) {
    echo "$key = ";
    $box = $value as Box<_>;
    echo $box->data;
    echo "\n";
  }

  echo "Testing F\n";
  echo F::A->data;
  echo "\n";
  echo F::C->data;
  echo "\n";
  echo "Testing F's getValues()\n";
  foreach (F::getValues() as $key => $value) {
    echo "$key = ";
    $box = $value as Box<_>;
    echo $box->data;
    echo "\n";
  }
}
