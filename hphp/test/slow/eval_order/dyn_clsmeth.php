<?hh

class Foo {
  public static function bar($baz) {
    echo "done\n";
  }
}

function foo() { echo "foo\n"; return "foo"; }
function bar() { echo "bar\n"; return "bar"; }
function baz() { echo "baz\n"; return "baz"; }

<<__EntryPoint>>
function main() {
  foo()::{bar()}(baz());
}
