<?hh
<<file:__EnableUnstableFeatures('readonly')>>
function test(bool $b, Exception $e, readonly Exception $e2): noreturn {
  if($b) {
    throw $e2; // error, cannot throw
  } else {
    throw $e; // ok
  }
}
