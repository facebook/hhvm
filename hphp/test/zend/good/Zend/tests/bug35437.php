<?hh

function err2exception($errno, $errstr)
:mixed{
    throw new Exception("Error occuried: " . $errstr);
}

class TestClass
{
    function testMethod()
:mixed    {
        \HH\global_set('t', new stdClass);
    }
}
<<__EntryPoint>> function main(): void {
set_error_handler(err2exception<>);

try {
    TestClass::testMethod();
} catch (Exception $e) {
    echo "Catched: ".$e->getMessage()."\n";
}
}
