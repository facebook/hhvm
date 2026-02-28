<?hh

function foo() :mixed{
  $u = "abc";
  $v = "\0";
  $w = "def\n";
  $x = $u . $v . $w;
  echo $x;
  echo "abc" . "\0" . "def\n";
}

<<__EntryPoint>>
function main_1579() :mixed{
foo();
$u = "abc";
$v = "\0";
$w = "def\n";
$x = $u . $v . $w;
echo $x;
echo "abc" . "\0" . "def\n";
echo "ab\0c\n";
}
