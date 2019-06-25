<?hh

abstract final class AutoloadStatics {
  public static $i = 0;
}
function __autoload($name)
{
    echo "IN:  " . __METHOD__ . "($name)\n";
    if (AutoloadStatics::$i++ > 10) {
        echo "-> Recursion detected - as expected.\n";
        return;
    }

    class_exists('UndefinedClass' . AutoloadStatics::$i);

    echo "OUT: " . __METHOD__ . "($name)\n";
}
<<__EntryPoint>> function main(): void {
var_dump(class_exists('UndefinedClass0'));
}
