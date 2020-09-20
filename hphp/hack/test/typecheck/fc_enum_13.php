<?hh // strict

// No using HH\BuiltinEnum
function foo(mixed $foo): bool {
  return $foo is HH\BuiltinEnum;
}
