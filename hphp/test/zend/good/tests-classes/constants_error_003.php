<?php
  class aclass
  {
      const myConst = "hello";
  }
  
  function f(&$a)
  {
      $a = "changed";
  }
  
  f(aclass::myConst);
  var_dump(aclass::myConst);
?>