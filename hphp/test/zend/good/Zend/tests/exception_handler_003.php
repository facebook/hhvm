<?hh

class test {

    function foo () :mixed{
        set_exception_handler(vec[$this, "bar"]);
    }

    <<__DynamicallyCallable>> function bar($e) :mixed{
        var_dump(get_class($e)." thrown!");
    }
}
<<__EntryPoint>> function main(): void {
$a = new test;
$a->foo();
throw new Exception();

echo "Done\n";
}
