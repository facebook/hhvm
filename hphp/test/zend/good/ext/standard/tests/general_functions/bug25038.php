<?hh

function bar($x='no argument')
{
    throw new Exception("This is an exception from bar({$x}).");
}
<<__EntryPoint>> function main(): void {
try
{
    bar('first try');
}
catch (Exception $e)
{
    print $e->getMessage()."\n";
}
try
{
    call_user_func(fun('bar'),'second try');
}
catch (Exception $e)
{
    print $e->getMessage()."\n";
}

echo "===DONE===\n";
}
