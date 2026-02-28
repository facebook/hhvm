<?hh

newtype MixedObj = mixed;

function foo(MixedObj $obj) :mixed{
}


<<__EntryPoint>>
function main_typedef_mixed_obj() :mixed{
foo(new stdClass);
foo(12);
echo "done\n";
}
