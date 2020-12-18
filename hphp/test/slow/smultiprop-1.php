<?hh

class C {
  static vec<int> $x = vec[];
  static vec<Foo> $y = vec[];
}

class Foo {}

<<__EntryPoint>>
function main() {
  C::$x[] = 17;
  C::$y[] = new Foo();

  gc_collect_cycles();

  var_dump(C::$x);
  var_dump(C::$y);
}
