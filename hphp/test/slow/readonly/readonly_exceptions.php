<?hh
<<file:__EnableUnstableFeatures("readonly")>>

function test(readonly Exception $e): noreturn {
  throw $e;
}
