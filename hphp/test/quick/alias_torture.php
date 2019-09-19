<?hh

<<__EntryPoint>>
function foo() {

  $b = $GLOBALS; // This should fatal.
  invariant_violation('should be unreachable');
}
