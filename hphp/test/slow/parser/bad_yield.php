<?hh

class C {
  function __construct() {
    yield 1;
  }
}


<<__EntryPoint>>
function main_bad_yield() {
echo "Done\n";
}
