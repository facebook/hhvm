<?hh

set_error_handler(function () {
    var_dump($x);
});

function foo() {
  $x = 42;
  return varray[@$x];
}

var_dump(foo());
@bar();
