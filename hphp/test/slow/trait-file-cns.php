<?hh

trait T { function foo() { var_dump(__FILE__); } }

<<__EntryPoint>>
function main() {
  include __FILE__.'.inc';

  (new C)->foo();
}
