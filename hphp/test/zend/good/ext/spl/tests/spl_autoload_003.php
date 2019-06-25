<?hh

function TestFunc1($classname)
{
    echo __METHOD__ . "($classname)\n";
}

function TestFunc2($classname)
{
    echo __METHOD__ . "($classname)\n";
    throw new Exception("Class $classname missing");
}

function TestFunc3($classname)
{
    echo __METHOD__ . "($classname)\n";
}
<<__EntryPoint>> function main(): void {
spl_autoload_register("TestFunc1");
spl_autoload_register("TestFunc2");
spl_autoload_register("TestFunc3");

try
{
    var_dump(class_exists("TestClass", true));
}
catch(Exception $e)
{
    echo 'Exception: ' . $e->getMessage() . "\n";
}

echo "===DONE===\n";
}
