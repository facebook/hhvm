<?hh

function foo(): void {
     $x = 5;
     // don't inline $x because used more than once
     $y = $x;
     var_dump(/*range-start*/$x/*range-end*/);
}
