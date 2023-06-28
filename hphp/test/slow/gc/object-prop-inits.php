<?hh

class C {
  public ImmSet $x = ImmSet { 1, 2, 3 };
  public ImmSet $y = ImmSet { 1, 2, 3 };
}

<<__EntryPoint>>
function main() :mixed{
  new C();
  gc_collect_cycles();
  $obj = new C();
  var_dump($obj->x, $obj->y);
}
