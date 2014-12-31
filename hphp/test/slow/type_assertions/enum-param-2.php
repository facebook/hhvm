<?hh

function a() {
  include_once __DIR__.'/enum-param.php';
}

function b() {
  class NotAnObject {
    const FOO = 2;
  }
}

function test2(NotAnObject $o) {
  var_dump($o);
}


a();  // define it as an enum in this request
test2(NotAnObject::FOO);
