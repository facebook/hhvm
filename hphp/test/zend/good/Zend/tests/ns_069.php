<?hh

namespace foo;

class Test {
  static function f() :mixed{
    \var_dump((string)__NAMESPACE__);
    include __DIR__ . '/ns_069.inc';
    \foo();
    \var_dump((string)__NAMESPACE__);
  }
}
<<__EntryPoint>> function main(): void {
Test::f();

echo "===DONE===\n";
}
