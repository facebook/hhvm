<?hh

class test {

    function foo () {
        set_exception_handler(varray[$this, "bar"]);
    }

    <<__DynamicallyCallable>> function bar($e) {
        var_dump(get_class($e)." thrown!");
    }
}
<<__EntryPoint>> function main(): void {
$a = new test;
$a->foo();
throw new Exception();

echo "Done\n";
}
