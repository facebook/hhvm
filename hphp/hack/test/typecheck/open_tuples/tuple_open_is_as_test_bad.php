<?hh
<<file: __EnableUnstableFeatures('open_tuples')>>

function test1(mixed $m): string {
  $m as (nonnull...);
  if ($m is (int, optional string)) {
    return $m[1] ?? "default";
  }
  if ($m is (int, string...)) {
    return $m[1] ?? "default";
  }
  $m as (optional bool, optional int);
  return "fail";
}
