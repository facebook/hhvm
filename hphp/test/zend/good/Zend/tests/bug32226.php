<?hh

class A
{
        public function __construct()
        {
                set_exception_handler(varray[$this, 'EH']);

                throw new Exception();
        }

        public function EH()
        {
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
