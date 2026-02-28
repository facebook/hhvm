<?hh

class c {
  function x() :mixed{
    var_dump($this);
  }
}

<<__EntryPoint>>
function main_732() :mixed{
$x = new c;
$x->x();
}
