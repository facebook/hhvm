<?hh

function foo() :mixed{
  $x = 42;
  return vec[@$x];
}
<<__EntryPoint>>
function entrypoint_fatal_and_no_op(): void {

  set_error_handler(function () {
      var_dump($x);
  });

  var_dump(foo());
  @bar();
}
