<?php

// Invalid class name
try {
	new ReflectionParameter (array ('A', 'b'), 0);
} catch (ReflectionException $e) { echo $e->getMessage ()."\n"; }

// Invalid class method
try {
	new ReflectionParameter (array ('C', 'b'), 0);
} catch (ReflectionException $e) { echo $e->getMessage ()."\n"; }

// Invalid object method
try {
	new ReflectionParameter (array (new C, 'b'), 0);
} catch (ReflectionException $e) { echo $e->getMessage ()."\n"; }

echo "Done.\n";

class C {
}

?>
