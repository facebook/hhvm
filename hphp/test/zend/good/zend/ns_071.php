<?php

namespace foo;

class bar {
	public function __construct(array $x = NULL) {
		var_dump($x);
	}
}

new bar(null);
new bar(new \stdclass);

?>