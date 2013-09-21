<?php
function obh($s)
{
	print_r($s, 1);
}
ob_start("obh");
echo "foo\n";
?>