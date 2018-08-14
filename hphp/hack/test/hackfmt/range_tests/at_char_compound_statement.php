<?hh

function foo(int $x) {
  takeSomeAction ( );
  {
    // UNSAFE_BLOCK
    takesString ( $x );
  }
  takeOtherAction ( );
}
