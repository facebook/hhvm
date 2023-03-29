<?hh

function foo(): void {
  // The curly braces in the param complicate our expression lambda check.
  // Make sure we don't crash.
  (($a = () ==> {}) ==>
    /*range-start*/3 + 3/*range-end*/
  );
}
