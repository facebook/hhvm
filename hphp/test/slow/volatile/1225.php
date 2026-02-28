<?hh

function foo($a) :mixed{
  if ($a) {
    include '1225-1.inc';
  }
}
function bar() :mixed{
  if (interface_exists('A')) {
    include '1225-2.inc';
    $obj = new C;
    var_dump($obj);
  }
 else {
    var_dump('no');
  }
}

<<__EntryPoint>>
function main_1225() :mixed{
foo(true);
bar();
}
