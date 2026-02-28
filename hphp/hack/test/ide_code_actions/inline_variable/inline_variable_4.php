<?hh

function foo(): void {
     $y = 3;
     $x = $y
     $y = 6;
     // inlining $x would change behavior because $y was reassigned
     var_dump(/*range-start*/$x/*range-end*/);
}
