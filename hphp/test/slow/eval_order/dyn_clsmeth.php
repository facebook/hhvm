<?hh

class Foo {
  public static function bar($baz) :mixed{
    echo "done\n";
  }
}

function bar() :mixed{ echo "bar\n"; return "bar"; }
function baz() :mixed{ echo "baz\n"; return "baz"; }

<<__EntryPoint>>
function main() :mixed{
  Foo::{bar()}(baz());
}
