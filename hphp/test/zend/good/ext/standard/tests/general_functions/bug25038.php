<?php

function bar($x='no argument')
{
    throw new Exception("This is an exception from bar({$x}).");
}
try
{
	bar('first try');
}
catch (Exception $e)
{
	print $e->getMessage()."\n";
}
try
{
	call_user_func('bar','second try');
}
catch (Exception $e)
{
	print $e->getMessage()."\n";
}

?>
===DONE===