<?hh

class A {
    <<__LSB>> private static string $x;
}

class B extends A {
    <<__LSB>> private static string $x;
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
