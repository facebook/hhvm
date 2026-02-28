<?hh

class X {
  public function a() :mixed{
 var_dump(get_class_methods($this));
 }
  protected function b() :mixed{
}
  private function c() :mixed{
}
  final function d() :mixed{
}
  public function e() :mixed{
}
}
class Y {
  public function a() :mixed{
 var_dump(get_class_methods($this));
 }
}

<<__EntryPoint>>
function main_1334() :mixed{
$x = new X;
$x->a();
$y = new Y;
$y->a();
var_dump(get_class_methods($x));
var_dump(get_class_methods($y));
}
