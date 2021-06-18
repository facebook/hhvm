<?hh

newtype MixedObj = mixed;

function foo(MixedObj $obj) {
}


<<__EntryPoint>>
function main_typedef_mixed_obj() {
foo(new stdClass);
foo(12);
echo "done\n";
}
