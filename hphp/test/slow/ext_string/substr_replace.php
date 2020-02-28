<?hh



<<__EntryPoint>>
function main_substr_replace() {
error_reporting(-1);
var_dump(substr_replace(varray["x", "y"], varray[], array(), 0));
var_dump(substr_replace(varray["x", "y"], varray[], 0, array()));
var_dump(substr_replace(varray["x", "y"], varray[], array(), array()));
var_dump(substr_replace(varray["x", "y"], varray[], 0, 0));

var_dump(substr_replace(varray["x", "y"], "z", varray[], 0));
var_dump(substr_replace(varray["x", "y"], "z", 0, varray[]));
var_dump(substr_replace(varray["x", "y"], "z", varray[], array()));
var_dump(substr_replace(varray["x", "y"], "z", 0, 0));
}
