<?hh

function foo($a) :mixed{
  $flag = true;
  try {
    f($a);
    $flag = false;
  }
 catch (Exception $e) {
  }
  var_dump($flag);
}
function f($a) :mixed{
 if ($a) throw new Exception('What?');
 }

<<__EntryPoint>>
function main_64() :mixed{
foo(1);
}
