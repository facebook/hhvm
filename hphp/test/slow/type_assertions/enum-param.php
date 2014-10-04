<?hh

enum NotAnObject : string as string {
  FOO = "Foobar";
}

function test(NotAnObject $o) {
  var_dump($o);
}

test(NotAnObject::FOO);
