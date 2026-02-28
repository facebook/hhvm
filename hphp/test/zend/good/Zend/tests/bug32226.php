<?hh

class A
{
        public function __construct()
        {
                set_exception_handler(vec[$this, 'EH']);

                throw new Exception();
        }

        public function EH()
:mixed        {
                restore_exception_handler();

                throw new Exception();
        }
}
<<__EntryPoint>> function main(): void {
try
{
$a = new A();
}
catch(Exception $e)
{
    echo "Caught\n";
}

echo "===DONE===\n";
}
