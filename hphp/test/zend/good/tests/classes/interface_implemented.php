<?php

interface if_a {
	function f_a();
}
	
interface if_b extends if_a {
	function f_b();
}

class base {
	function _is_a($sub) {
		echo 'is_a('.get_class($this).', '.$sub.') = '.(($this instanceof $sub) ? 'yes' : 'no')."\n";
	}
	function test() {
		echo $this->_is_a('base');
		echo $this->_is_a('derived_a');
		echo $this->_is_a('derived_b');
		echo $this->_is_a('derived_c');
		echo $this->_is_a('derived_d');
		echo $this->_is_a('if_a');
		echo $this->_is_a('if_b');
		echo "\n";
	}
}

class derived_a extends base implements if_a {
	function f_a() {}
}

class derived_b extends base implements if_a, if_b {
	function f_a() {}
	function f_b() {}
}

class derived_c extends derived_a implements if_b {
	function f_b() {}
}

class derived_d extends derived_c {
}

$t = new base();
$t->test();

$t = new derived_a();
$t->test();

$t = new derived_b();
$t->test();

$t = new derived_c();
$t->test();

$t = new derived_d();
$t->test();

?>