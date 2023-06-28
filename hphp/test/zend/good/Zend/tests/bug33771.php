<?hh

function make_exception()
:mixed{
    throw new Exception();
}

function make_exception_and_change_err_reporting()
:mixed{
    error_reporting(E_ALL & ~E_STRICT);
    throw new Exception();
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL | E_STRICT);

var_dump(error_reporting());

try {
    @make_exception();
} catch (Exception $e) {}

var_dump(error_reporting());

try {
    @make_exception_and_change_err_reporting();
} catch (Exception $e) {}

var_dump(error_reporting());

echo "Done\n";
}
