<?hh

<<file: __EnableUnstableFeatures('coeffects_provisional')>>

function vec_map<Te>(
  vec<int> $v,
  (function (int)[Te]: int) $f
)[Te]: vec<int> {
  $vo = vec[];
  foreach ($v as $i) {
    $vo[] = $f($i);
  }
  return $vo;
}

function hof_fail<Te>(
  (function (int)[Te]: int) $f
)[mixed]: void {
  $f(4); // Te not in scope
}

function plus_one_pure(int $i)[pure]: int {
  return $i + 1;
}

function plus_one_impure(int $i): int {
  echo "hello";
  return $i + 1;
}

function pure_caller(vec<int> $v)[pure]: void {
  vec_map($v, fun('plus_one_pure'));

  vec_map($v, fun('plus_one_impure')); // error
}

function impure_caller(vec<int> $v): void {
  vec_map($v, fun('plus_one_pure'));

  vec_map($v, fun('plus_one_impure'));
}
