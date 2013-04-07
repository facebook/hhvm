<?php
interface constr
{
	function __construct();
}

abstract class implem implements constr
{
}

class derived extends implem
{
	function __construct($a)
	{
	}
}

?>