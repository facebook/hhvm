<?hh
function f (...$a) : int {} // no error
function f (...$a, $b) : int {} // error
function f ($a, ...$b) : int {} // no error
function f ($a, ...$b, $c) : int {} // error
f ($a, ...$b); //no error
f (...$a); // no error
f (...$a, $b); // error
f ($a, ...$b, $c); // error
