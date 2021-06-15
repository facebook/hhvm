<?hh

class A implements StringishObject{
  function __toString() { return "I'M A"; }
}

class B implements StringishObject{
  function __toString() { return "I'M B"; }
}

function stringish_cast($value): string {
  return is_object($value) && $value is Stringish
    ? $value->__toString()
    : (string)$value;
}

function my_strtolower($x) {
  return strtolower($x);
}

function strtolower_wrapper($str): string {
  return my_strtolower(stringish_cast($str));
}

<<__EntryPoint>> function main(): void {
  $obja = new A;
  $objb = new B;
  var_dump(strtolower_wrapper($obja));
  var_dump(strtolower_wrapper($objb));
  var_dump(strtolower_wrapper($objb));
}
