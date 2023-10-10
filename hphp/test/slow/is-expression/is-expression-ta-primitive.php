<?hh

type Tbool = bool;
type Tint = int;
type Tfloat = float;
type Tstring = string;
type Tresource = resource;

function main(mixed $x): void {
  if ($x is Tbool) {
    echo "bool\n";
  }
  if ($x is Tint) {
    echo "int\n";
  }
  if ($x is Tfloat) {
    echo "float\n";
  }
  if ($x is Tstring) {
    echo "string\n";
  }
  if ($x is Tresource) {
    echo "resource\n";
  }
}


<<__EntryPoint>>
function main_is_expression_ta_primitive() :mixed{
main(true);
main(0);
main(1.5);
main("foo");
main(fopen(__FILE__, 'r'));
}
