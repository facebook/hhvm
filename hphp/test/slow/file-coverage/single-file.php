<?hh

function cover_me1() {
  $x = __hhvm_intrinsics\launder_value("hello, world!");
  $y = $x."~~";
  var_dump($y);
}

function cover_me2(bool $has_cover) {
  $x = __hhvm_intrinsics\launder_value("coverme!");
  if (!$has_cover) {
    $y = __hhvm_intrinsics\launder_value("dontcoverme");
    var_dump($y);
  }
  var_dump($x);
  if ($has_cover) {
    $z = __hhvm_intrinsics\launder_value("surprise!");
    var_dump($z."!!");
  }
}

function cover_me3() {
  $x = __hhvm_intrinsics\launder_value("hello, world!");
  $y = $x."!!";
  var_dump($y);
}

function cover_me4() {
  $x = __hhvm_intrinsics\launder_value("hello, world!");
  $y = $x."??";
  var_dump($y);
}

function print_cover_map(vec<int> $map, $max) {
  $first = -1;
  $last = -1;
  foreach ($map as $line) {
    if ($line > $max) break;
    if ($last + 1 !== $line) {
      if ($last !== $first)  echo "-$last\n";
      else if ($last !== -1) echo "\n";
      echo "$line";
      $first = $line;
    }
    $last = $line;
  }
  echo "\n";
}

<<__EntryPoint>>
function main() {
  cover_me1();      cover_me1();       // opt
  cover_me2(false); cover_me2(false);  // partial opt
  cover_me3();                         // profile
  HH\enable_per_file_coverage(keyset[__FILE__]);
  cover_me1();
  cover_me2(true);
  cover_me3();
  cover_me4();
  $map = HH\get_coverage_for_file(__FILE__);
  HH\disable_all_coverage();
  print_cover_map($map, 48);
}
