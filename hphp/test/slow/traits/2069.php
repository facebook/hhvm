<?hh

trait T {
  public function doIt() :mixed{
    throw new Exception('foobar');
  }
}
class X {
 use T;
 }
function f($obj) :AsyncGenerator<mixed,mixed,void>{
  $res = null;
  try {
    $res = $obj->doIt();
  }
 catch (Exception $e) {
    $res = $e->getMessage();
  }
  yield $res;
}

<<__EntryPoint>>
function main_2069() :mixed{
$x = new X;
foreach (f($x) as $i) {
 var_dump($i);
 }
}
