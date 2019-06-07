<?hh
function __autoload($class)
{
    eval("namespace ns_test; class test {}");

    throw new \Exception('abcd');
}
<<__EntryPoint>> function main() {
try
{
    \ns_test\test::go();
}
catch (Exception $e)
{
    echo 'caught';
}
}
