<?hh
class X {
  public static $foo;
}
function test() :mixed{
  $x = new X;
  $foo = 'foo';
  if (isset($x->$foo) || !($x->$foo ?? false) ||      isset($x->{
'bar'}
)) {
    unset($x->$foo);
    unset($x->{
'bar'}
);
    echo true;
  }
}


<<__EntryPoint>>
function main_1293() :mixed{
$data = new stdClass;
$type = 'OCI-Lob';
if (is_a($data, $type)) {
  echo 'true';
}
;
}
