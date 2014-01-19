<?php
function obh($s)
{
	return ob_get_flush();
}
ob_start("obh");
echo "foo\n";
?>