<?hh

class try_class
{
    static public function main ()
    {
        register_shutdown_function (varray ["self", "on_shutdown"]);
    }

    static public function on_shutdown ()
    {
        printf ("CHECKPOINT\n"); /* never reached */
    }
}
<<__EntryPoint>> function main(): void {
try_class::main ();

echo "Done\n";
}
