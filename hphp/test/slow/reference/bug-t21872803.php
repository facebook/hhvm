<?hh

  function wat(varray<int> $arr, bool $sort): vec<int> {
    if ($sort) {
      usort(inout $arr, function($a, $b) {
          return $a < $b ? -1 : ($a > $b ? 1 : 0);
        });
    }
    $arr = array_filter(
      $arr,
          $x ==> $x != 3
    );
    return vec($arr);
  }

function main() :mixed{
  var_dump(wat(vec[6, 3, 7, 1, 8], true));
}


<<__EntryPoint>>
function main_bug_t21872803() :mixed{
main();
}
