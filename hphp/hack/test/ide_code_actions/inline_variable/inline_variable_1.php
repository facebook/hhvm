<?hh

function foo(): void {
     $x = 0;
     var_dump(/*range-start*/$x/*range-end*/);
}
