<?hh
namespace Foo;
function f($a=darray[namespace\bar=>0]) {
  foreach ($a as $k => $v) { return $k; }
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
