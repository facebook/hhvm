<?hh

class C {
  function __construct() {
    function foo() {}
    yield foo();
  }
}


<<__EntryPoint>>
function main_bad_yield() {
echo "Done\n";
}
