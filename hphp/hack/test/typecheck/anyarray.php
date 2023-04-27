<?hh //strict

function takes_array(AnyArray<int, int> $_): void {}
function takes_container(Container<int> $_): void {}
function takes_keyed_container(KeyedContainer<int, int> $_): void {}
function gives_array(): AnyArray<int, int> { return vec[]; }
function handles_generic_array<Tk as arraykey, Tv>(
  AnyArray<Tk, Tv> $a,
): AnyArray<Tk, Tv> { return $a; }
function takes_reified_array<reify TParam>(): void {}
function expect_int(int $_): void {}

function does_things(): void {
  takes_array(varray[]);
  takes_array(darray[]);
  takes_array(vec[]);
  takes_array(dict[]);
  takes_array(keyset[]);
  takes_array(gives_array());
  takes_container(gives_array());
  handles_generic_array(handles_generic_array(gives_array()));

  $a = gives_array();
  $d = dict[];
  foreach ($a as $k => $v) {
    $d[$k] = $v;
  }
  takes_array($d);

  if ($a) {}

  takes_reified_array<AnyArray<string, mixed>>();

  if (HH\is_vec_or_varray($a)) {
    foreach($a as $k => $_) {
      expect_int($k);
    }
  }
}

interface Parent_ {
  public function foo(AnyArray<int, int> $f): void;
  public function bar(): AnyArray<arraykey, mixed>;
}

class Child implements Parent_ {
  public function foo(AnyArray<arraykey, int> $f): void {}
  public function bar(): AnyArray<arraykey, mixed> { return vec[]; }
}

class Child2 extends Child {
  public function bar(): AnyArray<int, string> { return dict[]; }
}

class Child3 extends Child2 {
  public function bar(): dict<int, string> { return dict[]; }
}

function takes_any_array(AnyArray<arraykey, mixed> $a): void {}

function m(mixed $m): void {
  if (HH\is_any_array($m)) {
    hh_show($m);
    takes_any_array($m);
  } else if (HH\is_vec_or_varray($m)) {
    takes_any_array($m);
  } else if (HH\is_vec_or_varray($m)) {
    takes_any_array($m);
  } else if ($m is dict<_,_>) {
    takes_any_array($m);
  } else if ($m is vec<_>) {
    takes_any_array($m);
  } else if ($m is keyset<_>) {
    takes_any_array($m);
  } else if ($m is AnyArray<_,_>) {
    hh_show($m);
    takes_any_array($m);
  }
}

<<__Memoize>>
function memoized(AnyArray<int, AnyArray<string, string>> $a): void {}

function purefn(AnyArray<int, string> $a): void {
  $a[42];
  foreach ($a as $k => $v) {}
}
