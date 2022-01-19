<?hh

function foo<reify T>() { T::bar(); }
class C { public static function bar() { echo "ok\n"; } }

<<__EntryPoint>>
function main() {
  foo<C>();
}
