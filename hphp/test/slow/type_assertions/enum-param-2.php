<?hh

function a() {
  include_once __DIR__.'/enum-param.php';
}

function b() {
  include 'enum-param-2.inc';
}

function test2(NotAnObject $o) {
  var_dump($o);
}
<<__EntryPoint>>
function entrypoint_enumparam2(): void {


  a();  // define it as an enum in this request
  test2(NotAnObject::FOO);
}
