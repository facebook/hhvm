<?hh

<<__EntryPoint>>
function main() {
  $varrays = vec[
    varray[],
    varray[1],
    varray[-1],
    varray['1'],
    varray[1, 1],
    varray[2],
  ];
  foreach ($varrays as $v1) {
    foreach ($varrays as $v2) {
      print('Comparing '.json_encode($v1).' and '.json_encode($v2).":\n");
      print('  <  '.($v1 < $v2 ? 'Y' : 'N')."\n");
      print('  <= '.($v1 <= $v2 ? 'Y' : 'N')."\n");
      print('  >  '.($v1 > $v2 ? 'Y' : 'N')."\n");
      print('  >= '.($v1 >= $v2 ? 'Y' : 'N')."\n");
    }
  }
}
