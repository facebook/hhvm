<?hh

<<__EntryPoint>>
function main_next() {
$transport = varray["foot", "bike", "car", "plane"];
var_dump(current($transport));
var_dump(next(inout $transport));
var_dump(next(inout $transport));
var_dump(prev(inout $transport));
var_dump(end(inout $transport));
}
