<?php
class X {
  static $foo;
}
function test() {
  $x = new X;
  $foo = 'foo';
  if (isset($x->$foo) || empty($x->$foo) ||      isset($x->{
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
$data = new stdclass;
$type = 'OCI-Lob';
if ($data instanceof $type) {
  echo 'true';
}
;
}
