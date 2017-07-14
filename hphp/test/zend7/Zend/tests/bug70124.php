<?php 

try  {
	echo base_convert([array_search(chr(48),chr(48),chr(48),chr(48),chr(48),$f("test"))],chr(48));
} catch (Error $e) {
	var_dump($e->getMessage());
}

class A {
}

try  {
	echo base_convert([array_search(chr(48),chr(48),chr(48),chr(48),chr(48),a::y("test"))],chr(48));
} catch (Error $e) {
	var_dump($e->getMessage());
}

$a = new A;

try  {
	echo base_convert([array_search(chr(48),chr(48),chr(48),chr(48),chr(48),$a->y("test"))],chr(48));
} catch (Error $e) {
	var_dump($e->getMessage());
}

try  {
	echo base_convert([array_search(chr(48),chr(48),chr(48),chr(48),chr(48),\bar\y("test"))],chr(48));
} catch (Error $e) {
	var_dump($e->getMessage());
}

try  {
	echo base_convert([array_search(chr(48),chr(48),chr(48),chr(48),chr(48),y("test"))],chr(48));
} catch (Error $e) {
	var_dump($e->getMessage());
}
?>
