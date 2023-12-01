<?hh

namespace HH\Lib\DictLib;

function foo(): dict<int, int> {
  $d = dict(dict[1 => 1]);
  hh_show($d);
  return $d;
}
