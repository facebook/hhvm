<?hh

set_error_handler(function () {
    var_dump($x);
  });

function foo() {
  $x = 42;
  return array(@$x);
}

var_dump(foo());
@bar();
