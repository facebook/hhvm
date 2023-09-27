<?hh

<<file:__EnableUnstableFeatures('case_types')>>

class A {}

function test1(string $x): void {
}

case type C1 = string | int;
function test1c(C1 $x): void {
}

function test2(arraykey $x): void {
}

case type C2 = arraykey | int;
function test2c(C2 $x): void {
}

function test3(classname<A> $x): void {
}

case type C3 = classname<A> | int;
function test3c(C3 $x): void {
}

<<__EntryPoint>>
function main() :mixed{
  require "test.inc";
  throw_errors();

  test1(A::class);
  test1c(A::class);
  echo "---\n";
  test2(A::class);
  test2c(A::class);
  echo "---\n";
  test3(A::class);
  test3c(A::class);

  echo "Done\n";
}
