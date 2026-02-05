<?hh

function err($errno, $errstr, $errfile, $errline)
:mixed{
        throw new Exception($errstr);
}

class test {
    function foo() :mixed{
        $this->blah->prop = "string";
        $var = $this->blah->prop;
        var_dump($this->blah);
    }
}
<<__EntryPoint>> function main(): void {
set_error_handler(err<>);

$t = new test;
try {
    $t->foo();
} catch (Exception $e) {
    var_dump($e->getMessage());
}

echo "Done\n";
}
