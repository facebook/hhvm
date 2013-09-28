<?php

class a { function a() { var_dump("a::a() called"); } } 
class b extends a {} 

$b = new b; 
var_dump(is_callable(array($b,"a")));
var_dump(is_callable(array($b,"b")));
var_dump(is_callable(array($b,"__construct")));

echo "Done\n";
?>