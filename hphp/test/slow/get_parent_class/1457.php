<?php

class dad {
  function dad()  {
}
}
class child extends dad {
  function child()  {
    echo "I'm " , get_parent_class($this) , "'s son\n";
  }
}
class child2 extends dad {
  function child2()  {
    echo "I'm " , get_parent_class('child2') , "'s son too
";
  }
}
$foo = new child();
$bar = new child2();
