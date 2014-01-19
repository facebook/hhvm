<?php
echo "*** Test by calling method or function with its expected arguments and output to variable ***\n";
$foo = 'bar';
$baz = "something ".$foo."\n";

if ( $foo == 'bar' ) 
{
  $baz = "baz\n";
}

 /* some code here */ 
$source = show_source(__FILE__, true);

var_dump($source);
?>