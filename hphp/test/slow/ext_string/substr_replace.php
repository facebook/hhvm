<?hh



<<__EntryPoint>>
function main_substr_replace() {
error_reporting(-1);
var_dump(substr_replace(array("x", "y"), array(), array(), 0));
var_dump(substr_replace(array("x", "y"), array(), 0, array()));
var_dump(substr_replace(array("x", "y"), array(), array(), array()));
var_dump(substr_replace(array("x", "y"), array(), 0, 0));

var_dump(substr_replace(array("x", "y"), "z", array(), 0));
var_dump(substr_replace(array("x", "y"), "z", 0, array()));
var_dump(substr_replace(array("x", "y"), "z", array(), array()));
var_dump(substr_replace(array("x", "y"), "z", 0, 0));
}
