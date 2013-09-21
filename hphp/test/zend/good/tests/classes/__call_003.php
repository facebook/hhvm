<?php
  class C
  {
      function __call($name, $values)
      {
          $values[0][0] = 'changed';
      }
  }
  
  $a = array('original');
  
  $b = array('original');
  $hack =& $b[0];
  
  $c = new C;
  $c->f($a);
  $c->f($b);
  
  var_dump($a, $b);
?>