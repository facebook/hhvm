<?hh

class try_class
{
    static public function main ()
:mixed    {
        register_shutdown_function (vec["self", "on_shutdown"]);
    }

    static public function on_shutdown ()
:mixed    {
        printf ("CHECKPOINT\n"); /* never reached */
    }
}
<<__EntryPoint>> function main(): void {
try_class::main ();

echo "Done\n";
}
