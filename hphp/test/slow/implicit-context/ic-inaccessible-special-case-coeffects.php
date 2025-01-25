<?hh

// we are matching the coeffect requiments for MakeICInaccessible, so leak_safe or less is no allowed
<<__Memoize(#ICInaccessibleSpecialCase)>>
function memo_inaccessible_sc_leaksafe($a, $b)[leak_safe]: mixed{
  echo "memo_inaccessible_sc_leaksafe: $a, $b \n";
}

<<__EntryPoint>>
function main() {
  memo_inaccessible_sc_leaksafe(1, 2);
}
