<?hh



<<__EntryPoint>>
function main_substr_replace() :mixed{
error_reporting(-1);
var_dump(substr_replace(vec["x", "y"], vec[], vec[], 0));
var_dump(substr_replace(vec["x", "y"], vec[], 0, vec[]));
var_dump(substr_replace(vec["x", "y"], vec[], vec[], vec[]));
var_dump(substr_replace(vec["x", "y"], vec[], 0, 0));

var_dump(substr_replace(vec["x", "y"], "z", vec[], 0));
var_dump(substr_replace(vec["x", "y"], "z", 0, vec[]));
var_dump(substr_replace(vec["x", "y"], "z", vec[], vec[]));
var_dump(substr_replace(vec["x", "y"], "z", 0, 0));
}
