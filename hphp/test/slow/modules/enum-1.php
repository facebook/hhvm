<?hh

module a;

function dyn($x, $y, $enum_class = false) :mixed{
  try {
    var_dump($x::BAR);
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
  try {
    var_dump($enum_class ? EC::$y : Foo::$y);
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main() :mixed{
  include "enum-1.inc";
  try {
    var_dump(Foo::BAR);
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
  var_dump(SoftFoo::BAR);
  try {
    var_dump(EC::BAR);
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
  var_dump(SoftEC::BAR);

  echo "Dynamic>>\n\n";
  dyn("Foo", "BAR");
  dyn("SoftFoo", "BAR");
  dyn("EC", "BAR", true);
  dyn("SoftEC", "BAR", true);
}
