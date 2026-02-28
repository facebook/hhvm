<?hh

class dad {
  function __construct()  {
}
}
class child extends dad {
  function __construct()  {
    echo "I'm " , get_parent_class($this) , "'s son\n";
  }
}
class child2 extends dad {
  function __construct()  {
    echo "I'm " , get_parent_class('child2') , "'s son too
";
  }
}

<<__EntryPoint>>
function main_1457() :mixed{
$foo = new child();
$bar = new child2();
}
