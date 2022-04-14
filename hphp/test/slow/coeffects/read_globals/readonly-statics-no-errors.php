<?hh

class Foo {
  public static int $i = 0;
}

function globals_no_error()[globals] : void {
  $y = readonly Foo::$i;
}

function read_globals_no_error()[read_globals] : void {
  $y = readonly Foo::$i;
}

<<__EntryPoint>>
function main(): void {
  globals_no_error();
  read_globals_no_error();
  echo "Done\n";
}
