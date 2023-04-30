<?hh

interface Catchable
{
}

class MyException extends Exception implements Catchable
{
    function __construct($errstr, $errno, $errfile, $errline)
    {
        parent::__construct($errstr, $errno);
        $this->file = $errfile;
        $this->line = $errline;
    }
}

function Error2Exception($errno, $errstr, $errfile, $errline)
{
    throw new MyException($errstr, $errno, $errfile, $errline);
}
<<__EntryPoint>> function main(): void {
$err_msg = 'no exception';
set_error_handler(Error2Exception<>);

try
{
    $con = fopen(sys_get_temp_dir().'/a_file_that_does_not_exist','r');
}
catch (Catchable $e)
{
    echo "Catchable\n";
}
catch (Exception $e)
{
    echo "Exception\n";
}

echo "===DONE===\n";
}
