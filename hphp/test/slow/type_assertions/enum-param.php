<?hh

enum NotAnObject : string as string {
  FOO = "Foobar";
}

function test(NotAnObject $o) :mixed{
  var_dump($o);
}
<<__EntryPoint>>
function entrypoint_enumparam(): void {

  test(NotAnObject::FOO);
}
