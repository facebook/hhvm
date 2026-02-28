<?hh

function foo<reify T>() :mixed{ T::bar(); }
class C { public static function bar() :mixed{ echo "ok\n"; } }

<<__EntryPoint>>
function main() :mixed{
  foo<C>();
}
