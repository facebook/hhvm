<?hh


<<__EntryPoint>>
function main_prev() {
$transport = array("foot", "bike", "car", "plane");
var_dump(current($transport));
var_dump(next(inout $transport));
var_dump(next(inout $transport));
var_dump(prev(inout $transport));
var_dump(end(inout $transport));
}
