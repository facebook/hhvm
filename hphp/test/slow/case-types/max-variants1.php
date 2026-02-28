<?hh

<<file:__EnableUnstableFeatures('case_types')>>

class A1 {}
class A2 {}
class A3 {}
class A4 {}
class A5 {}
class A6 {}
class A7 {}
class A8 {}

case type C1 = A1 | A2 | A3 | A4 | A5 | A6 | A7 | A8;

function foo(C1 $a): void {
  echo "Test should not get here\n";
}

<<__EntryPoint>>
function main(): void {
  foo(new A1());
}
