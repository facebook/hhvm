<?hh

class Foo {
  use Bar,Baz,Quz{
    Bar::bar insteadof Baz;Baz::baz insteadof Bar,Quz;Quz::quz as quz;
  }
}
