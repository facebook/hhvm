<?hh
function f (...$a) : int {} // no error
function f (...$a, $b) : int {} // ERROR
function f ($a, ...$b) : int {} // no error
function f ($a, ...$b, $c) : int {} // ERROR
f ($a, ...$b); //no error
f (...$a); // no error
f (...$a, $b); // ERROR
f ($a, ...$b, $c); // ERROR
f ($a, ...$b, ); // no error
