<?hh

namespace Foo;

<<__EntryPoint>>
function main_throwable_namespace_001() {
$x = new \Exception();
\var_dump($x is \Throwable);
\var_dump($x is Throwable);
}
