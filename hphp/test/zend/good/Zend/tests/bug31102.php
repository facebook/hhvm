<?hh

abstract final class ZendGoodZendTestsBug31102 {
  public static $test = 0;
}

function __autoload($class)
{


    echo __METHOD__ . "($class,".ZendGoodZendTestsBug31102::$test.")\n";
    switch(ZendGoodZendTestsBug31102::$test)
    {
    case 1:
        eval("class $class { function __construct(){throw new Exception('$class::__construct');}}");
        return;
    case 2:
        eval("class $class { function __construct(){throw new Exception('$class::__construct');}}");
        throw new Exception(__METHOD__);
        return;
    case 3:
        return;
    }
}
<<__EntryPoint>> function main(): void {
while(ZendGoodZendTestsBug31102::$test++ < 5)
{
    try
    {
        eval("\$bug = new Test".ZendGoodZendTestsBug31102::$test."();");
    }
    catch (Exception $e)
    {
        echo "Caught: " . $e->getMessage() . "\n";
    }
}
echo "===DONE===\n";
}
