<?php

interface showable
{
	static function show();
}

class pass implements showable
{
	static function show() {
		echo "Call to function show()\n";
	}
}

pass::show();

eval('
class fail
{
	abstract static function func();
}
');

fail::show();

echo "Done\n"; // shouldn't be displayed
?>