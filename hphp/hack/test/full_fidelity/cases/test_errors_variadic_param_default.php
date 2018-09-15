<?hh
function f (...$a = null) : int {} // ERROR
function f ($a, ...$b = null) : int {} // ERROR
function f ($a = null, ...$b) : int {} // no error
