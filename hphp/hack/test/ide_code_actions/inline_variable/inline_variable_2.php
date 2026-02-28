<?hh

function foo(): void {
     $x = 10 + 20;
     var_dump(/*range-start*/$x/*range-end*/);
}
