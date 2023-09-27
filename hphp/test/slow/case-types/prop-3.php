<?hh

<<file: __EnableUnstableFeatures('case_types')>>

case type C1 = int | string;
case type C2 = int | string;

class A {
  <<__LateInit>> public C1 $a;
}

class B extends A {
  <<__LateInit>> public C2 $a;
}

<<__EntryPoint>>
function main(): void {
  require "test.inc";

  $b = new B();

  echo "done\n";
}
