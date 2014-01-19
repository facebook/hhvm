<?php

interface if_a {
	function f_a();
}
	
interface if_b extends if_a {
	function f_b();
}

class base {
	function _is_a($sub) {
		
		echo "\n>>> With Defined class\n";
		echo str_pad('is_a( OBJECT:'.get_class($this).', '.$sub.') = ', 60) . (is_a($this, $sub) ? 'yes' : 'no')."\n";
		echo str_pad('is_a( STRING:'.get_class($this).', '.$sub.') = ', 60). (is_a(get_class($this), $sub) ? 'yes' : 'no')."\n";
		echo str_pad('is_a( STRING:'.get_class($this).', '.$sub.', true) = ', 60). (is_a(get_class($this), $sub, true) ? 'yes' : 'no')."\n";		
		echo str_pad('is_subclass_of( OBJECT:'.get_class($this).', '.$sub.') = ', 60).  (is_subclass_of($this, $sub) ? 'yes' : 'no')."\n";
		echo str_pad('is_subclass_of( STRING:'.get_class($this).', '.$sub.') = ', 60). (is_subclass_of(get_class($this), $sub) ? 'yes' : 'no')."\n";
		echo str_pad('is_subclass_of( STRING:'.get_class($this).', '.$sub.',false) = ', 60). (is_subclass_of(get_class($this), $sub , false) ? 'yes' : 'no')."\n";
		 
		// with autoload options..
		echo ">>> With Undefined\n";
		echo  str_pad('is_a( STRING:undefB, '.$sub.',true) = ', 60). (is_a('undefB', $sub, true) ? 'yes' : 'no')."\n";
		echo  str_pad('is_a( STRING:undefB, '.$sub.') = ', 60). (is_a('undefB', $sub) ? 'yes' : 'no')."\n";
		echo  str_pad('is_subclass_of( STRING:undefB, '.$sub.',false) = ', 60). (is_subclass_of('undefB', $sub, false) ? 'yes' : 'no')."\n";
		echo  str_pad('is_subclass_of( STRING:undefB, '.$sub.') = ', 60). (is_subclass_of('undefB', $sub) ? 'yes' : 'no')."\n";
	}
	function test() {
		echo $this->_is_a('base');
		echo $this->_is_a('derived_a');  
		echo $this->_is_a('if_a'); 
		echo $this->_is_a('undefA');
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

eval('
  function __autoload($name)
  {
      echo ">>>> In __autoload: ";
      var_dump($name);
  }
');

echo "NOW WITH AUTOLOAD\n\n";

$t = new base();
$t->test();

$t = new derived_a();
$t->test();

$t = new derived_b();
$t->test();
 




?>