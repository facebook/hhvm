<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

// A collection class with constraint
interface IMem {
  public function Equal(mixed $x): bool;
}

class MySet<+T as IMem> {
  public function __construct(private Vector<T> $items) {}
  public function Union<Tu super T as IMem>(MySet<Tu> $that): MySet<Tu> {
    $newitems = Vector {};
    foreach ($this->items as $x) {
      $newitems[] = $x;
    }
    foreach ($that->items as $x) {
      $newitems[] = $x;
    }
    return new MySet($newitems);
  }
  public function Member(mixed $elem): bool {
    foreach ($this->items as $x) {
      if ($x->Equal($elem))
        return true;
    }
    return false;
  }
}

class MyString implements IMem {
  public function __construct(private string $item) {}
  public function Equal(mixed $x): bool {
    return $this->item === $x;
  }
}

class Test {
  public function Doit(): MySet<MyString> {
    $x = new MySet(Vector { new MyString('a'), new MyString('b') });
    return $x;
  }
}
