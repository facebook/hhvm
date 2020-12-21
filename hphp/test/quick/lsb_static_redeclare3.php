<?hh

class A {
    <<__LSB>> protected static string $x;
}

class B extends A {
    <<__LSB>> protected static string $x;
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
