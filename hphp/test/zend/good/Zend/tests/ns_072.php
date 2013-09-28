<?php

namespace foo;

interface foo {
	
}

class bar {
	public function __construct(foo $x = NULL) {
		var_dump($x);
	}
}

class test implements foo {
	
}


new bar(new test);
new bar(null);
new bar(new \stdclass);

?>