<?hh

function e() :mixed{
 return 'hello';
 }
function foo() :mixed{
  $expected = e();
  $list_expected = "[$expected,$expected]";
  var_dump($expected, $list_expected);
}

<<__EntryPoint>>
function main_1706() :mixed{
foo();
}
