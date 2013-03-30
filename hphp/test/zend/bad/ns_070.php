<?php

namespace foo;

class bar {
	public function __construct(\stdclass $x = NULL) {
		var_dump($x);
	}
}

new bar(new \stdclass);
new bar(null);

?>