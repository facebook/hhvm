<?hh

function err($errno, $errstr, $errfile, $errline)
{
        throw new Exception($errstr);
}

class test {
    function foo() {
        $var = $this->blah->prop = "string";
        var_dump($this->blah);
    }
}
<<__EntryPoint>> function main(): void {
set_error_handler(fun("err"));

$t = new test;
try {
    $t->foo();
} catch (Exception $e) {
    var_dump($e->getMessage());
}

echo "Done\n";
}
