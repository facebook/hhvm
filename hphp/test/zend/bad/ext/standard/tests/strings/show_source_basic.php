<?php
echo "*** Test by calling method or function with its expected arguments ***\n";
$foo = 'bar';
$baz = "something ".$foo."\n";

if ( $foo == 'bar' ) 
{
  $baz = 'baz';
}

 /* some code here */
   
show_source(__FILE__);

?>