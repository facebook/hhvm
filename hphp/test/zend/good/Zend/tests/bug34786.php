<?hh
function foo($a,$b,$c) :mixed{
  echo "foo: ".error_reporting()."\n";
}

function bar() :mixed{
  echo "bar: ".error_reporting()."\n";
}

<<__EntryPoint>> function main(): void {
error_reporting(1);
echo "before: ".error_reporting()."\n";
@foo(1,@bar(),3);
echo "after: ".error_reporting()."\n";
}
