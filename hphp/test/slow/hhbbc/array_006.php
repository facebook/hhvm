<?hh

function randint() :mixed{
  if (mt_rand() < 100) return 12;
  if (mt_rand() < 1000) return 120;
  return 53;
}

function pt2d() :mixed{
  return dict['x' => randint(), 'y' => randint()];
}
function pt3d() :mixed{
  return dict['x' => randint(), 'y' => randint(), 'z' => randint()];
}

function test(bool $x) :mixed{
  $z = $x ? pt2d() : pt3d();
  return $z['y'];
}

function main() :mixed{
  $x = test(true);
  var_dump(is_int($x));
  $x = test(false);
  var_dump(is_int($x));
}


<<__EntryPoint>>
function main_array_006() :mixed{
main();
}
