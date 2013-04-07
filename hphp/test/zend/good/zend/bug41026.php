<?php

class try_class
{
	static public function main ()
	{
		register_shutdown_function (array ("self", "on_shutdown"));
	}

	static public function on_shutdown ()
	{
		printf ("CHECKPOINT\n"); /* never reached */
	}
}

try_class::main ();

echo "Done\n";
?>