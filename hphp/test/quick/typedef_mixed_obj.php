<?hh

newtype MixedObj = mixed;

function foo(MixedObj $obj) {
}

foo(new stdclass);
foo(12);
echo "done\n";
