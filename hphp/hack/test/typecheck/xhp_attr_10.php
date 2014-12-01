<?hh // strict
class :foo {
  // Typechecker should complain about a type mismatch between the
  // typehint and the default value
  attribute bool x = "yar";
}
