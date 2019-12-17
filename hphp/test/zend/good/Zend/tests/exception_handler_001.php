<?hh

function foo($e) {
    var_dump(get_class($e)." thrown!");
}

class test extends Exception {
}
<<__EntryPoint>> function main(): void {
set_exception_handler(fun("foo"));

throw new test();

echo "Done\n";
}
