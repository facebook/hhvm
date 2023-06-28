<?hh

module a.b;

function dyn($x, $y, $enum_class) :mixed{
  try {
    var_dump($x::FOO);
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
  try {
    var_dump($enum_class ? EnumClsFoo::$y : EnumFoo::$y);
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main_enum_throw_1() :mixed{
  try {
    var_dump(EnumFoo::FOO);
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
  try {
    var_dump(EnumClsFoo::FOO);
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }

  echo "Dynamic>>\n\n";
  dyn("EnumFoo", "FOO", false);
  dyn("EnumClsFoo", "FOO", true);
}
