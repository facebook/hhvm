<?hh

function foo() {
  $x = 42;
  return varray[@$x];
}
<<__EntryPoint>>
function entrypoint_fatal_and_no_op(): void {

  set_error_handler(function () {
      var_dump($x);
  });

  var_dump(foo());
  @bar();
}
