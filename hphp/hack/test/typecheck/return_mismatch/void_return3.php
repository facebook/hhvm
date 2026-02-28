<?hh

function returnsvoid() : void {}

// Quite inconsistently, return foo; with foo : void is
// disallowed in functions that have a return type hint,
// but is allowed if the function doesn't have one
function test() : void {

  // accepted
  $lam1 = ()  ==> {
    return returnsvoid();
  };

  // not accepted
  $lam2 = () : void ==> {
    return returnsvoid();
  };
}
