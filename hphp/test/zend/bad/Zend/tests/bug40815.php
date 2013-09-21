<?php

class ehandle{
	static public function exh ($ex) {
		echo 'foo';
	}
}

set_exception_handler("ehandle::exh");

throw new Exception ("Whiii");
echo "Done\n";
?>