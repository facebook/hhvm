<?hh

class C {
  public static vec<int> $x = vec[];
  public static vec<Foo> $y = vec[];
}

class Foo {}

<<__EntryPoint>>
function main() :mixed{
  C::$x[] = 17;
  C::$y[] = new Foo();

  gc_collect_cycles();

  var_dump(C::$x);
  var_dump(C::$y);
}
