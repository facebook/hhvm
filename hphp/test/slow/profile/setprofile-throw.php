<?hh

function foo($t) :mixed{ echo "From $t: This shouldn't happen!\n"; }

class X {
  public static function foo() :mixed{
    try {
      return true;
    } catch (Exception $e) {
      foo(static::class);
      return false;
    }
  }
}

function main($f) :mixed{
  try {
    // If we call X::foo directly here, hhbbc will see an effect-free
    // function call, and optimize it away.
    // This is not a bug - we intentionally consider functions that
    // can only have side effects via surprise hooks to be effect
    // free.
    $f();
  } catch (Exception $e) {
    echo "Exception with message: ", $e->getMessage(), "\n";
  }
}


<<__EntryPoint>>
function main_setprofile_throw() :mixed{
fb_setprofile(function($what, $fun) {
    if ($what == 'exit' && $fun == 'X::foo') {
      echo "Throwing from $fun\n";
      throw new Exception("Surprise!");
    }
  });

main('X::foo');
}
