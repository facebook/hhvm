<?hh

abstract class foo {
  <<__Memoize>>
  abstract protected static function bar():mixed;
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
