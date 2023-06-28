<?hh

function make_exception()
:mixed{
    @$blah;
    str_replace();
    error_reporting(0);
    throw new Exception();
}
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

try {
    @make_exception();
} catch (Exception $e) {}

var_dump(error_reporting());

error_reporting(E_ALL&~E_NOTICE);

try {
    @make_exception();
} catch (Exception $e) {}

var_dump(error_reporting());

echo "Done\n";
}
