<?hh

function foo($t) { echo "From $t: This shouldn't happen!\n"; }

fb_setprofile(function($what, $fun) {
    if ($what == 'exit' && $fun == 'X::foo') {
      echo "Throwing from $fun\n";
      throw new Exception("Surprise!");
    }
  });

class X {
  public function foo() {
    try {
      return true;
    } catch (Exception $e) {
      foo(get_class($this));
      return false;
    }
  }
}

function main($f) {
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

main('X::foo');
