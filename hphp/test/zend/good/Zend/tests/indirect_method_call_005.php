<?php

class foo extends ArrayObject {
	public function __construct($arr) {
		parent::__construct($arr);
	}
}

var_dump( (new foo( array(1, array(4, 5), 3) ))[1][0] ); // int(4)

?>