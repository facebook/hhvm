<?php

trait foo {
	public function serialize() {
		return 'foobar';
	}
	public function unserialize($x) {
		var_dump($x);
	}
}

class bar implements Serializable {
	use foo;
}

var_dump($o = serialize(new bar));
var_dump(unserialize($o));

?>
