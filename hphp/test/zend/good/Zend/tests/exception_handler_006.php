<?hh

function foo($e) {
    var_dump(__FUNCTION__."(): ".get_class($e)." thrown!");
}

function foo1($e) {
    var_dump(__FUNCTION__."(): ".get_class($e)." thrown!");
}

<<__EntryPoint>> function main(): void {
set_exception_handler(foo<>);
set_exception_handler(foo1<>);

restore_exception_handler();

throw new Exception();

echo "Done\n";
}
