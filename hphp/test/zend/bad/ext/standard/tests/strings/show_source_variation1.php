<?php
echo "*** Test by calling method or function with its expected arguments and php output ***\n";
$foo = 'bar';
$baz = "something ".$foo."\n";

if ( $foo == 'bar' ) 
{
  $baz = "baz\n";
}

 /* some code here */
echo $baz;   
show_source(__FILE__);
echo $foo;
?>