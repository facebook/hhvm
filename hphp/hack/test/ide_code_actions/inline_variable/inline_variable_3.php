<?hh

function foo(): void {
     $y = 3;
     $x = $y
     var_dump(/*range-start*/$x/*range-end*/);
}
