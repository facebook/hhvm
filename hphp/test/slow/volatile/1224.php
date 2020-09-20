<?hh

function foo($a) {
  if ($a) {
    include '1224-1.inc';
  }
}
function bar() {
  if (class_exists('A')) {
    include '1224-2.inc';
    $obj = new C;
    var_dump($obj);
  }
 else {
    var_dump('no');
  }
}

<<__EntryPoint>>
function main_1224() {
foo(true);
bar();
}
