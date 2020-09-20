<?hh

class X {
}

function test($x) {
  call_user_func(varray[$x, "FOO"]);
}

function main() {
  try {
    test(new X);
  } catch (Exception $e) {
    echo "Exception\n";
  }
}


<<__EntryPoint>>
function main_cuf_throw() {
set_error_handler(function() { throw new Exception; });

main();
echo "Done\n";
}
