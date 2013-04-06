<?php

function do_throw() {
	throw new Exception();
}

class aa 
{
	function check()
	{
	}

	function dosome()
	{
		$this->check(do_throw());
	}
}
$l_aa=new aa();

$l_aa->dosome();
?>