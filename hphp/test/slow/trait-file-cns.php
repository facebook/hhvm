<?hh

trait T {
  function foo() {
    var_dump(__FILE__);
    var_dump(__DIR__);
    if (substr(__FILE__, 0, strlen(__DIR__)) === __DIR__) {
      echo "Okay\n";
    } else {
      echo "Fail\n";
    }
  }
}

<<__EntryPoint>>
function main() {
  include __FILE__.'.inc';

  (new C)->foo();
}
