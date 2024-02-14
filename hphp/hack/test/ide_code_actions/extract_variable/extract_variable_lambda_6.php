<?hh

function foo(): void {
  (($a = () ==> {}) ==>
    /*range-start*/3 + 3/*range-end*/
  );
}
