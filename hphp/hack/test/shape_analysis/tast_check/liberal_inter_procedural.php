<?hh

function f(mixed $_, dict<string, mixed> $_, mixed $_): void {}

class C {
  public function m(mixed $_, dict<string, mixed> $_, mixed $_): void {}

  public static function s(mixed $_, dict<string, mixed> $_, mixed $_): void {}
}

class D {
  public function m(mixed $_, dict<string, mixed> $_, mixed $_): void {}
}

function test_function(): void {
  $d1 = dict[]; // Not invalidated
  $d2 = dict[]; // Invalidated
  $d3 = dict[]; // Not invalidated
  f($d1, $d2, $d3);
}

function test_method(C $c): void {
  $d1 = dict[]; // Not invalidated
  $d2 = dict[]; // Invalidated
  $d3 = dict[]; // Not invalidated
  $c->m($d1, $d2, $d3);
}

function test_class_method(): void {
  $d1 = dict[]; // Not invalidated
  $d2 = dict[]; // Invalidated
  $d3 = dict[]; // Not invalidated
  C::s($d1, $d2, $d3);
}

function test_union(C $c, D $d): void {
  $d1 = dict[]; // Not invalidated
  $d2 = dict[]; // Invalidated
  $d3 = dict[]; // Not invalidated

  $o = 1 === 2 ? $c : $d;
  $o->m($d1, $d2, $d3);
}
