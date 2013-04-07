<?php
interface constr1
{
	function __construct();
}

interface constr2 extends constr1
{
}

class implem12 implements constr2
{
	function __construct()
	{
	}
}

interface constr3
{
	function __construct($a);
}

class implem13 implements constr1, constr3
{
	function __construct()
	{
	}
}
?>