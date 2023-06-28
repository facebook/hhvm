<?hh


<<__EntryPoint>>
function main_225() :mixed{
$a = darray['a' => 1, 'b' => 2];
foreach ($a as $b => $c) {
  var_dump($b);
  unset($a['b']);
}
}
