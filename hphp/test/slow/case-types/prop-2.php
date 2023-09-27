<?hh

<<file: __EnableUnstableFeatures('case_types')>>

case type C1 = int | string;
case type C2 = int | string | float;

class A {
  <<__LateInit>> public C1 $a;
}

class B extends A {
  <<__LateInit>> public C2 $a;
}

<<__EntryPoint>>
function main(): void {
  $b = new B();

  echo "done\n";
}
