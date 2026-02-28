<?hh

class ErrorHandling
{
    <<__DynamicallyCallable>> public function errorHandler1($errno, $errstr)
:mixed    {
        echo "Caught on first level: '$errstr'\n";
        return true;
    }

    <<__DynamicallyCallable>> public function errorHandler2($errno, $errstr)
:mixed    {
        echo "Caught on second level: '$errstr'\n";
        return true;
    }
}

function errorHandler1($errno, $errstr)
:mixed{
    echo "Caught on first level: '$errstr'\n";
    return true;
}

function errorHandler2($errno, $errstr)
:mixed{
    echo "Caught on second level: '$errstr'\n";
    return true;
}
<<__EntryPoint>> function main(): void {
$err = new ErrorHandling();

set_error_handler(vec[$err, 'errorHandler1']);
set_error_handler(vec[$err, 'errorHandler2']);

trigger_error('Foo', E_USER_WARNING);

set_error_handler(errorHandler1<>);
set_error_handler(errorHandler2<>);

trigger_error('Foo', E_USER_WARNING);

echo "==END==";
}
