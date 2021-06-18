<?hh
class X {
  static $foo;
}
function test() {
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
function main_1293() {
$data = new stdClass;
$type = 'OCI-Lob';
if (is_a($data, $type)) {
  echo 'true';
}
;
}
