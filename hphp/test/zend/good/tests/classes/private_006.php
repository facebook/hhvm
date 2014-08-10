<?php
class first {
	private static function show() {
		echo "Call show()\n";
	}

	public static function do_show() {
		first::show();
	}
}

first::do_show();

class second extends first { 
}

second::do_show();

class third extends second {
}

third::do_show();

class fail extends third {
	static function show() {  // cannot be redeclared
		echo "Call show()\n";
	}
}

echo "Done\n";
?>
