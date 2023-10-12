<?hh // strict
class :foo {
  // This file is strict, so the typecheck should complain about
  // an unbound name
  attribute NonExistentClass x;
}
