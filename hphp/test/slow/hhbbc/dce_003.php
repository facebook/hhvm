<?hh

function heh($ar) :mixed{
  if (!$ar) throw new Exception('a');
  return 42;
}
function bar($ar) :mixed{
  $tmp = 54;
  try {
    $tmp = heh($ar);
  } catch (Exception $x) {
    var_dump($tmp);
  }
  var_dump($tmp);
}


<<__EntryPoint>>
function main_dce_003() :mixed{
bar(vec['a']);
bar(vec[]);
}
