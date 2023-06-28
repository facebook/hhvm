<?hh
<<__DynamicallyCallable>>
function global_func()
:mixed{
    echo __METHOD__ . "\n";
}

class foo
{
    public static $method = 'global_func';
    <<__DynamicallyCallable>>
    static public function foo_func()
:mixed    {
        echo __METHOD__ . "\n";
    }
}

<<__EntryPoint>> function main(): void {
$function = 'global_func';
$function();

/* The following is a BC break with PHP 4 where it would
 * call foo::fail. In PHP 5 we first evaluate static class
 * properties and then do the function call.
 */
$method = 'foo_func';
foo::$method();

echo "===DONE===\n";
}
