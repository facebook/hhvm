<?hh

function f (int $a) : Vector<array> {} // error
function f (array $a) : int {} // error
function f (int $a) : array {} // error
function f (array<int> $a) : array<int, string> {} //no error
