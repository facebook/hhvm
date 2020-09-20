<?hh

function test($a) {
  $f = function() use($a) {
    yield 1;
    yield 2;
  };

  foreach ($f() as $v) {
    var_dump($v);
  }
}


<<__EntryPoint>>
function main_unused() {
test("unused");
}
