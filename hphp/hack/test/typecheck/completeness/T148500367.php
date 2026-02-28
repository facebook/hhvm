<?hh

class C {
  public ?int $prop;
}

function id(int $x): int { return $x; }

function incomplete1(vec<vec<int>> $v, C $obj): void {
  $obj->prop as nonnull;
  $v[id($obj->prop)][] = 1;
}

function incomplete2(vec<vec<int>> $v, C $obj): void {
  $obj->prop as nonnull;
  $v[id($obj->prop)][42] = 1;
}
