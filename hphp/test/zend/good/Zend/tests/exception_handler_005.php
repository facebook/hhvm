<?hh

set_exception_handler(fun("foo"));
set_exception_handler(fun("foo1"));

function foo($e) {
	var_dump(__FUNCTION__."(): ".get_class($e)." thrown!");
}

function foo1($e) {
	var_dump(__FUNCTION__."(): ".get_class($e)." thrown!");
}


throw new excEption();

echo "Done\n";
