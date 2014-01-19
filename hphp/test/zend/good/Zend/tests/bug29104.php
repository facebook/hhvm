<?php
class A
{ 
  function g() 
  { 
    echo "function g - begin\n";
    
    function f() 
    { 
      echo "function f\n";
    } 

    echo "function g - end\n";
  }
}

$a = new A;
$a->g();
f();
?>