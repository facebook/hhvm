<?hh

class X {
}

function test($x) :mixed{
  call_user_func(vec[$x, "FOO"]);
}

function main() :mixed{
  try {
    test(new X);
  } catch (Exception $e) {
    echo "Exception\n";
  }
}


<<__EntryPoint>>
function main_cuf_throw() :mixed{
set_error_handler(function() { throw new Exception; });

main();
echo "Done\n";
}
