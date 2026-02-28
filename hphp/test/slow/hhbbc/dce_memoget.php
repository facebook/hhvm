<?hh

<<__Memoize>>
function foo(string $a, ?string $b) :mixed{
  throw new Exception;
}

<<__EntryPoint>>
function main() :mixed{
  try {
    foo("hello", "world");
  } catch (Exception $e) {
    echo "Caught!\n";
  }
}
