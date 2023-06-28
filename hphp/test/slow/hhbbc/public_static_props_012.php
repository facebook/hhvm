<?hh

class Foo {
  public static $y = 'asd';
}

abstract class IDunno {
  abstract function x($z):mixed;
}
class A extends IDunno {
  function x(inout $z) :mixed{ $z = 2; }
}
class B extends IDunno {
  function x($z) :mixed{ $z = 2; }
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
