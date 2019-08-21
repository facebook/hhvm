<?hh

set_exception_handler(fun("foo"));

function foo($e) {
	var_dump(get_class($e)." thrown!");
}

class test extends Exception {
}

throw new test();

echo "Done\n";
