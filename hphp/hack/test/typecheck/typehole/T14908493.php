<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface IBoxedValue {
}

abstract class BoxedValue<T> implements IBoxedValue {
  public abstract function get(): T;
  public abstract function add(T $other): T;
}

final class BoxedInt extends BoxedValue<int> {
  public function __construct(private int $value) {
  }

  public function get(): int {
    return $this->value;
  }

  public function add(int $other): int {
    return $this->value + $other;
  }
}

final class BoxedString extends BoxedValue<string> {
  public function __construct(private string $value) {
  }

  public function get(): string {
    return $this->value;
  }

  public function add(string $other): string {
    return $this->value . $other;
  }
}

function map<Tv1, Tv2>(vec<Tv1> $in, (function(Tv1): Tv2) $fn): vec<Tv2> {
  $out = vec[];
  foreach ($in as $v) {
    $out[] = $fn($v);
  }
  return $out;
}

function add_first_two(vec<IBoxedValue> $values): mixed {
  $values = map(
    $values,
    $v ==> {
      $v as BoxedValue<_>;
      return $v;
    },
  );

  return $values[0]->add($values[1]->get());
}

<<__EntryPoint>>
function main():void {
  var_dump(add_first_two(vec[new BoxedInt(42), new BoxedString('hello')]));
}
