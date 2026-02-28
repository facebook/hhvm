<?hh

function foo($a) :mixed{
  if ($a) {
    include '1224-1.inc';
  }
}
function bar() :mixed{
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
function main_1224() :mixed{
foo(true);
bar();
}
