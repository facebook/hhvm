<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class A {
  public static function meth(): void {}
}

function dict_key(class<A> $c): void {
  $d = dict[$c => 1];
  foreach ($d as $k => $_) {
    $k::meth();
  }
}

function map_key(class<A> $c): void {
  $m = Map {$c => 1};
  foreach ($m as $k => $_) {
    $k::meth();
  }
}

function keyset_element(class<A> $c): void {
  foreach (keyset[$c] as $k) {
    $k::meth();
  }
}

function set_element(class<A> $c): void {
  foreach (Set {$c} as $k) {
    $k::meth();
  }
}
