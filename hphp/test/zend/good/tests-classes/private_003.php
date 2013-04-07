<?php
ini_set("error_reporting",2039);
class pass {
	private static function show() {
		echo "Call show()\n";
	}

	protected static function good() {
		pass::show();
	}
}

class fail extends pass {
	static function ok() {
		pass::good();
	}

	static function not_ok() {
		pass::show();
	}
}

fail::ok();
fail::not_ok(); // calling a private function

echo "Done\n"; // shouldn't be displayed
?>