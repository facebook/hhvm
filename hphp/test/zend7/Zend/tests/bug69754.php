<?php

class Example {
	public function test() {
		var_dump(static::class);
		var_dump(static::class . 'IsAwesome');
		var_dump(static::class . date('Ymd'));
		var_dump([static::class]);
    }
}

(new Example)->test();

?>
