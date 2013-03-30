<?php
function eh()
{
	throw new Exception("error!");
	return false;
}

set_error_handler("eh");
$a = $empty($b);