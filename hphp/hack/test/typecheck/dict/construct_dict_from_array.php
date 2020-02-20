<?hh // partial

namespace HH\Lib\DictLib;

function foo(): dict<int, int> {
  $d = dict(darray[1 => 1]);
  hh_show($d);
  return $d;
}
