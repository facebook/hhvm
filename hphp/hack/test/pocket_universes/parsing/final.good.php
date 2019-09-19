<?hh // strict

class Foo
{
  final enum Field {
    case type T;
    case string ident;
    case T default_value;
  }
}
