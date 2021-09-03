<?hh

class C {
  const type T1 = int;
  const type T = self::T1;
}

class D extends C {
  const type T1 = string;

  function f($x) { var_dump($x is this::T); }
}

<<__EntryPoint>>
function main() {
  (new D())->f(1);
}
