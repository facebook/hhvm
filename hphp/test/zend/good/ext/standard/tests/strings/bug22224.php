<?php
class foo
{
	function __toString()
	{
		return "Object";
	}
}


$a = new foo();
			    
$arr = array(0=>&$a, 1=>&$a);
var_dump(implode(",",$arr));
var_dump($arr)
?>