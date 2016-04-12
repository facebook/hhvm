<?hh

namespace HH\Lib\DictLib;

function foo(): dict<int, int> {
  $d = dict(array(1 => 1));
  hh_show($d);
  return $d;
}
