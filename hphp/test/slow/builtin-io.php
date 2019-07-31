<?hh

/*
 * function builtin_io(
 *   string $s,
 *   inout string $str,
 *   inout int $num,
 *   int $i,
 *   inout object $obj
 *   object $o,
 *   mixed $m,
 *   inout mixed $mix
 *   bool retOrig
 * ): array;
 */
function go(bool $ret) {
  echo "ret = $ret\n\n";

  $s1 = "hello";
  $s2 = "world";
  $i1 = 10;
  $i2 = 12;
  $o1 = new stdclass;
  $o2 = new Exception;
  $m1 = "mixed";
  $m2 = 42;
  var_dump(
    "one",
    "two",
    [1,2,3],
    $s1,
    $s2,
    $i1,
    $i2,
    array_map(
      $x ==> is_object($x) ? get_class($x) : $x,
      __hhvm_intrinsics\builtin_io(
        $s1,
        inout $s2,
        inout $i1,
        $i2,
        inout $o1,
        $o2,
        $m1,
        inout $m2,
        $ret,
      ),
    ),
  );
  var_dump(
    $s1,
    $s2,
    $i1,
    $i2,
    get_class($o1),
    get_class($o2),
    gettype($m1),
    gettype($m2),
  );
  echo "====================================================================\n";
}

/*
 * function builtin_io(
 *   string $s,
 *   inout string $str,
 *   inout int $num,
 *   int $i,
 *   inout object $obj
 *   object $o,
 *   mixed $m,
 *   inout mixed $mix
 *   bool retOrig
 * ): array;
 */
function go2(bool $ret) {
  echo "ret = $ret\n\n";

  $s1 = "hello";
  $s2 = "world";
  $i1 = 10;
  $i2 = 12;
  $o1 = new stdclass;
  $o2 = new Exception;
  $m1 = "mixed";
  $m2 = 42;
  var_dump(
    "one",
    "two",
    [1,2,3],
    $s1,
    $s2,
    $i1,
    $i2,
    array_map(
      $x ==> is_object($x) ? get_class($x) : $x,
      __hhvm_intrinsics\builtin_io_no_fca(
        $s1,
        inout $s2,
        inout $i1,
        $i2,
        inout $o1,
        $o2,
        $m1,
        inout $m2,
        $ret,
      ),
    ),
  );
  var_dump(
    $s1,
    $s2,
    $i1,
    $i2,
    get_class($o1),
    get_class($o2),
    gettype($m1),
    gettype($m2),
  );
  echo "====================================================================\n";
}

<<__EntryPoint>>
function main() {
  go(true);  go(false);  go(true);
  go2(true); go2(false); go2(true);
}
