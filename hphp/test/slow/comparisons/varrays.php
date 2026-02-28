<?hh

<<__EntryPoint>>
function main() :mixed{
  $varrays = vec[
    vec[],
    vec[1],
    vec[-1],
    vec['1'],
    vec[1, 1],
    vec[2],
  ];
  foreach ($varrays as $v1) {
    foreach ($varrays as $v2) {
      print('Comparing '.json_encode($v1).' and '.json_encode($v2).":\n");
      print('  <  '.(HH\Lib\Legacy_FIXME\lt($v1, $v2) ? 'Y' : 'N')."\n");
      print('  <= '.(HH\Lib\Legacy_FIXME\lte($v1, $v2) ? 'Y' : 'N')."\n");
      print('  >  '.(HH\Lib\Legacy_FIXME\gt($v1, $v2) ? 'Y' : 'N')."\n");
      print('  >= '.(HH\Lib\Legacy_FIXME\gte($v1, $v2) ? 'Y' : 'N')."\n");
    }
  }
}
