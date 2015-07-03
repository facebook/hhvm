<?hh

function f($x) { return $x . "my_string"; }
function x($x) { return Set { f($x), f($x) }; }
var_dump(is_object(x("asd".mt_rand())));
