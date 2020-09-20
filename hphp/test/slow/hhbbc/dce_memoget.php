<?hh // strict

<<__Memoize>>
function foo(string $a, ?string $b) {
  throw new Exception;
}

<<__EntryPoint>>
function main() {
  try {
    foo("hello", "world");
  } catch (Exception $e) {
    echo "Caught!\n";
  }
}
