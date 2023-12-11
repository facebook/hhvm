<?hh


<<__EntryPoint>>
function main_304() :mixed{
  $a = dict[0 => 'test'];
  $a[2] = vec[0];
  var_dump($a);

  $a = darray(vec['test']);
  $a[2] = vec[0];
  var_dump($a);
}
