<?hh

namespace foo;

class Test {
  static function f() {
    \var_dump((string)__NAMESPACE__);
    include __DIR__ . '/ns_069.inc';
    \var_dump((string)__NAMESPACE__);
  }
}
<<__EntryPoint>> function main(): void {
Test::f();

echo "===DONE===\n";
}
