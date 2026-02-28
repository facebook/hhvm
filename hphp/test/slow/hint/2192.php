<?hh
function decide() :mixed{

  return mt_rand(0, 0);
}


<<__EntryPoint>>
function main_2192() :mixed{

if (decide()) {
  include '2192-1.inc';
}
 else {
  include '2192-2.inc';
}
$x = new X;
foreach ($x->generator() as $v) {
  var_dump($v);
}
}
