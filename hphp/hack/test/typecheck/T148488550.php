<?hh

class C {
  public ?int $prop;
}

function id(int $x): int { return $x; }

function main(vec<int> $v, C $obj): void {
  $obj->prop as nonnull;
  $v[id($obj->prop)] += 1;
}
