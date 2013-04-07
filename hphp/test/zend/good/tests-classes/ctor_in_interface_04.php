<?php
interface constr
{
	function __construct();
}

class implem implements constr
{
	function __construct()
	{
	}
}

class derived extends implem
{
	function __construct($a)
	{
	}
}

?>