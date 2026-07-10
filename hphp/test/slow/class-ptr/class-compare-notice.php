<?hh

class Info { public static bool $SawError = false; }
function starts_with($x, $y): bool {
  $len = strlen($y);
  return !$len || 0 === strncmp($x, $y, $len);
}
function handle_error($_errno, $msg, ...$_rest) :mixed{
  //if (!Info::$SawError && $msg === "Class to string conversion") {
  if (starts_with($msg, "Implicit Class to string conversion")) {
    Info::$SawError = true;
    return true;
  } else if (
    $msg === "Comparing class and lazy class using a relational operator" ||
    $msg === "Comparing lazy class and string using a relational operator") {
    return true;
  }
  return false;
}

<<__DynamicallyReferenced>> class foobar {}

class StrObj {
  public function __construct(private string $s)[] {}
  public function __toString()[]: string { return $this->s; }
}
class Wrapper { public function __construct(private mixed $w)[] {} }

function LV($x)  :mixed{ return __hhvm_intrinsics\launder_value($x); }
function CLS($c) :mixed{ return HH\classname_to_class($c); }

function WRAPA($x) :mixed{ return LV(vec[$x]); }
function WRAPO($x) :mixed{ return LV(new Wrapper($x)); }
function WRAPD($x) :mixed{ $r = new stdClass; $r->x = $x; return LV($r); }

<<__NEVER_INLINE>> function print_header($title) :mixed{
  echo "$title\n";
  echo "+------------+------+------+------+------+------+------+\n";
  echo "| VAR        | <    | <=   | >    | >=   | ==   | ===  |\n";
  echo "+============+======+======+======+======+======+======+";
}
<<__NEVER_INLINE>> function begin_row($var, $wrap = null) :mixed{
  printf("\n| %-10s |", $wrap !== null ? $wrap."(\$$var)" : "\$$var");
}
<<__NEVER_INLINE>> function C(bool $v) :mixed{
  printf(" %-4s |", ($v ? 'T' : 'F').(Info::$SawError ? '*' : ''));
  Info::$SawError = false;
}
// Object coercions in comparisons now throw; render such cells as 'E'.
<<__NEVER_INLINE>> function CE((function(): bool) $f) :mixed{
  try {
    C($f());
  } catch (TypecastException $e) {
    printf(" %-4s |", 'E'.(Info::$SawError ? '*' : ''));
    Info::$SawError = false;
  }
}
<<__NEVER_INLINE>> function print_footer() :mixed{
  echo "\n+------------+------+------+------+------+------+------+\n\n";
}

<<__NEVER_INLINE>> function static_compare() :mixed{
  $cm = foobar::class;
  $va = 'foobar';
  $oa = new StrObj('foobar');
  $fa = CLS('foobar');

  $xx = vec[$cm]; $vx = vec[$va]; $ox = vec[$oa]; $fx = vec[$fa];

  $xy = new Wrapper($cm); $vy = new Wrapper($va); $oy = new Wrapper($oa);
  $fy = new Wrapper($fa);

  $xz = new stdClass; $xz->v = $cm; $vz = new stdClass; $vz->v = $va;
  $oz = new stdClass; $oz->v = $oa; $fz = new stdClass; $fz->v = $fa;

  print_header('[static] $cm ? VAR');
  begin_row('va');
    C($cm<$va);C($cm<=$va);C($cm>$va);C($cm>=$va);C($cm==$va);C($cm===$va);
  begin_row('oa');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($cm, $oa));CE(() ==> HH\Lib\Legacy_FIXME\lte($cm, $oa));CE(() ==> HH\Lib\Legacy_FIXME\gt($cm, $oa));CE(() ==> HH\Lib\Legacy_FIXME\gte($cm, $oa));CE(() ==> HH\Lib\Legacy_FIXME\eq($cm, $oa));C($cm===$oa);
  begin_row('fa');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($cm, $fa));CE(() ==> HH\Lib\Legacy_FIXME\lte($cm, $fa));CE(() ==> HH\Lib\Legacy_FIXME\gt($cm, $fa));CE(() ==> HH\Lib\Legacy_FIXME\gte($cm, $fa));CE(() ==> HH\Lib\Legacy_FIXME\eq($cm, $fa));C($cm===$fa);

  begin_row('va', 'WRAPA');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xx, $vx));CE(() ==> HH\Lib\Legacy_FIXME\lte($xx, $vx));CE(() ==> HH\Lib\Legacy_FIXME\gt($xx, $vx));CE(() ==> HH\Lib\Legacy_FIXME\gte($xx, $vx));CE(() ==> HH\Lib\Legacy_FIXME\eq($xx, $vx));C($xx===$vx);
  begin_row('oa', 'WRAPA');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xx, $ox));CE(() ==> HH\Lib\Legacy_FIXME\lte($xx, $ox));CE(() ==> HH\Lib\Legacy_FIXME\gt($xx, $ox));CE(() ==> HH\Lib\Legacy_FIXME\gte($xx, $ox));CE(() ==> HH\Lib\Legacy_FIXME\eq($xx, $ox));C($xx===$ox);
  begin_row('fa', 'WRAPA');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xx, $fx));CE(() ==> HH\Lib\Legacy_FIXME\lte($xx, $fx));CE(() ==> HH\Lib\Legacy_FIXME\gt($xx, $fx));CE(() ==> HH\Lib\Legacy_FIXME\gte($xx, $fx));CE(() ==> HH\Lib\Legacy_FIXME\eq($xx, $fx));C($xx===$fx);

  begin_row('va', 'WRAPO');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xy, $vy));CE(() ==> HH\Lib\Legacy_FIXME\lte($xy, $vy));CE(() ==> HH\Lib\Legacy_FIXME\gt($xy, $vy));CE(() ==> HH\Lib\Legacy_FIXME\gte($xy, $vy));CE(() ==> HH\Lib\Legacy_FIXME\eq($xy, $vy));C($xy===$vy);
  begin_row('oa', 'WRAPO');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xy, $oy));CE(() ==> HH\Lib\Legacy_FIXME\lte($xy, $oy));CE(() ==> HH\Lib\Legacy_FIXME\gt($xy, $oy));CE(() ==> HH\Lib\Legacy_FIXME\gte($xy, $oy));CE(() ==> HH\Lib\Legacy_FIXME\eq($xy, $oy));C($xy===$oy);
  begin_row('fa', 'WRAPO');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xy, $fy));CE(() ==> HH\Lib\Legacy_FIXME\lte($xy, $fy));CE(() ==> HH\Lib\Legacy_FIXME\gt($xy, $fy));CE(() ==> HH\Lib\Legacy_FIXME\gte($xy, $fy));CE(() ==> HH\Lib\Legacy_FIXME\eq($xy, $fy));C($xy===$fy);

  begin_row('va', 'WRAPD');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xz, $vz));CE(() ==> HH\Lib\Legacy_FIXME\lte($xz, $vz));CE(() ==> HH\Lib\Legacy_FIXME\gt($xz, $vz));CE(() ==> HH\Lib\Legacy_FIXME\gte($xz, $vz));CE(() ==> HH\Lib\Legacy_FIXME\eq($xz, $vz));C($xz===$vz);
  begin_row('oa', 'WRAPD');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xz, $oz));CE(() ==> HH\Lib\Legacy_FIXME\lte($xz, $oz));CE(() ==> HH\Lib\Legacy_FIXME\gt($xz, $oz));CE(() ==> HH\Lib\Legacy_FIXME\gte($xz, $oz));CE(() ==> HH\Lib\Legacy_FIXME\eq($xz, $oz));C($xz===$oz);
  begin_row('fa', 'WRAPD');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xz, $fz));CE(() ==> HH\Lib\Legacy_FIXME\lte($xz, $fz));CE(() ==> HH\Lib\Legacy_FIXME\gt($xz, $fz));CE(() ==> HH\Lib\Legacy_FIXME\gte($xz, $fz));CE(() ==> HH\Lib\Legacy_FIXME\eq($xz, $fz));C($xz===$fz);
  print_footer();

  print_header('[static] VAR ? $cm');
  begin_row('va');
    C($va<$cm);C($va<=$cm);C($va>$cm);C($va>=$cm);C($va==$cm);C($va===$cm);
  begin_row('oa');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($oa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\lte($oa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\gt($oa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\gte($oa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\eq($oa, $cm));C($oa===$cm);
  begin_row('fa');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($fa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\lte($fa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\gt($fa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\gte($fa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\eq($fa, $cm));C($fa===$cm);

  begin_row('va', 'WRAPA');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($vx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\lte($vx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\gt($vx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\gte($vx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\eq($vx, $xx));C($vx===$xx);
  begin_row('oa', 'WRAPA');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($ox, $xx));CE(() ==> HH\Lib\Legacy_FIXME\lte($ox, $xx));CE(() ==> HH\Lib\Legacy_FIXME\gt($ox, $xx));CE(() ==> HH\Lib\Legacy_FIXME\gte($ox, $xx));CE(() ==> HH\Lib\Legacy_FIXME\eq($ox, $xx));C($ox===$xx);
  begin_row('fa', 'WRAPA');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($fx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\lte($fx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\gt($fx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\gte($fx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\eq($fx, $xx));C($fx===$xx);

  begin_row('va', 'WRAPO');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($vy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\lte($vy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\gt($vy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\gte($vy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\eq($vy, $xy));C($vy===$xy);
  begin_row('oa', 'WRAPO');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($oy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\lte($oy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\gt($oy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\gte($oy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\eq($oy, $xy));C($oy===$xy);
  begin_row('fa', 'WRAPO');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($fy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\lte($fy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\gt($fy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\gte($fy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\eq($fy, $xy));C($fy===$xy);

  begin_row('va', 'WRAPD');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($vz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\lte($vz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\gt($vz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\gte($vz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\eq($vz, $xz));C($vz===$xz);
  begin_row('oa', 'WRAPD');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($oz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\lte($oz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\gt($oz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\gte($oz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\eq($oz, $xz));C($oz===$xz);
  begin_row('fa', 'WRAPD');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($fz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\lte($fz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\gt($fz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\gte($fz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\eq($fz, $xz));C($fz===$xz);
  print_footer();
}

<<__NEVER_INLINE>> function dynamic_compare() :mixed{
  $cm = LV(foobar::class);
  $va = LV('foobar');
  $oa = LV(new StrObj('foobar'));
  $fa = LV(CLS('foobar'));

  $xx = WRAPA($cm); $vx = WRAPA($va); $ox = WRAPA($oa); $fx = WRAPA($fa);

  $xy = WRAPO($cm); $vy = WRAPO($va); $oy = WRAPO($oa); $fy = WRAPO($fa);

  $xz = WRAPD($cm); $vz = WRAPD($va); $oz = WRAPD($oa); $fz = WRAPD($fa);

  print_header('[dynamic] $cm ? VAR');
  begin_row('va');
    C($cm<$va);C($cm<=$va);C($cm>$va);C($cm>=$va);C($cm==$va);C($cm===$va);
  begin_row('oa');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($cm, $oa));CE(() ==> HH\Lib\Legacy_FIXME\lte($cm, $oa));CE(() ==> HH\Lib\Legacy_FIXME\gt($cm, $oa));CE(() ==> HH\Lib\Legacy_FIXME\gte($cm, $oa));CE(() ==> HH\Lib\Legacy_FIXME\eq($cm, $oa));C($cm===$oa);
  begin_row('fa');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($cm, $fa));CE(() ==> HH\Lib\Legacy_FIXME\lte($cm, $fa));CE(() ==> HH\Lib\Legacy_FIXME\gt($cm, $fa));CE(() ==> HH\Lib\Legacy_FIXME\gte($cm, $fa));CE(() ==> HH\Lib\Legacy_FIXME\eq($cm, $fa));C($cm===$fa);

  begin_row('va', 'WRAPA');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xx, $vx));CE(() ==> HH\Lib\Legacy_FIXME\lte($xx, $vx));CE(() ==> HH\Lib\Legacy_FIXME\gt($xx, $vx));CE(() ==> HH\Lib\Legacy_FIXME\gte($xx, $vx));CE(() ==> HH\Lib\Legacy_FIXME\eq($xx, $vx));C($xx===$vx);
  begin_row('oa', 'WRAPA');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xx, $ox));CE(() ==> HH\Lib\Legacy_FIXME\lte($xx, $ox));CE(() ==> HH\Lib\Legacy_FIXME\gt($xx, $ox));CE(() ==> HH\Lib\Legacy_FIXME\gte($xx, $ox));CE(() ==> HH\Lib\Legacy_FIXME\eq($xx, $ox));C($xx===$ox);
  begin_row('fa', 'WRAPA');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xx, $fx));CE(() ==> HH\Lib\Legacy_FIXME\lte($xx, $fx));CE(() ==> HH\Lib\Legacy_FIXME\gt($xx, $fx));CE(() ==> HH\Lib\Legacy_FIXME\gte($xx, $fx));CE(() ==> HH\Lib\Legacy_FIXME\eq($xx, $fx));C($xx===$fx);

  begin_row('va', 'WRAPO');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xy, $vy));CE(() ==> HH\Lib\Legacy_FIXME\lte($xy, $vy));CE(() ==> HH\Lib\Legacy_FIXME\gt($xy, $vy));CE(() ==> HH\Lib\Legacy_FIXME\gte($xy, $vy));CE(() ==> HH\Lib\Legacy_FIXME\eq($xy, $vy));C($xy===$vy);
  begin_row('oa', 'WRAPO');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xy, $oy));CE(() ==> HH\Lib\Legacy_FIXME\lte($xy, $oy));CE(() ==> HH\Lib\Legacy_FIXME\gt($xy, $oy));CE(() ==> HH\Lib\Legacy_FIXME\gte($xy, $oy));CE(() ==> HH\Lib\Legacy_FIXME\eq($xy, $oy));C($xy===$oy);
  begin_row('fa', 'WRAPO');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xy, $fy));CE(() ==> HH\Lib\Legacy_FIXME\lte($xy, $fy));CE(() ==> HH\Lib\Legacy_FIXME\gt($xy, $fy));CE(() ==> HH\Lib\Legacy_FIXME\gte($xy, $fy));CE(() ==> HH\Lib\Legacy_FIXME\eq($xy, $fy));C($xy===$fy);

  begin_row('va', 'WRAPD');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xz, $vz));CE(() ==> HH\Lib\Legacy_FIXME\lte($xz, $vz));CE(() ==> HH\Lib\Legacy_FIXME\gt($xz, $vz));CE(() ==> HH\Lib\Legacy_FIXME\gte($xz, $vz));CE(() ==> HH\Lib\Legacy_FIXME\eq($xz, $vz));C($xz===$vz);
  begin_row('oa', 'WRAPD');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xz, $oz));CE(() ==> HH\Lib\Legacy_FIXME\lte($xz, $oz));CE(() ==> HH\Lib\Legacy_FIXME\gt($xz, $oz));CE(() ==> HH\Lib\Legacy_FIXME\gte($xz, $oz));CE(() ==> HH\Lib\Legacy_FIXME\eq($xz, $oz));C($xz===$oz);
  begin_row('fa', 'WRAPD');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($xz, $fz));CE(() ==> HH\Lib\Legacy_FIXME\lte($xz, $fz));CE(() ==> HH\Lib\Legacy_FIXME\gt($xz, $fz));CE(() ==> HH\Lib\Legacy_FIXME\gte($xz, $fz));CE(() ==> HH\Lib\Legacy_FIXME\eq($xz, $fz));C($xz===$fz);
  print_footer();

  print_header('[dynamic] VAR ? $cm');
  begin_row('va');
    C($va<$cm);C($va<=$cm);C($va>$cm);C($va>=$cm);C($va==$cm);C($va===$cm);
  begin_row('oa');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($oa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\lte($oa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\gt($oa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\gte($oa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\eq($oa, $cm));C($oa===$cm);
  begin_row('fa');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($fa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\lte($fa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\gt($fa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\gte($fa, $cm));CE(() ==> HH\Lib\Legacy_FIXME\eq($fa, $cm));C($fa===$cm);

  begin_row('va', 'WRAPA');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($vx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\lte($vx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\gt($vx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\gte($vx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\eq($vx, $xx));C($vx===$xx);
  begin_row('oa', 'WRAPA');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($ox, $xx));CE(() ==> HH\Lib\Legacy_FIXME\lte($ox, $xx));CE(() ==> HH\Lib\Legacy_FIXME\gt($ox, $xx));CE(() ==> HH\Lib\Legacy_FIXME\gte($ox, $xx));CE(() ==> HH\Lib\Legacy_FIXME\eq($ox, $xx));C($ox===$xx);
  begin_row('fa', 'WRAPA');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($fx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\lte($fx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\gt($fx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\gte($fx, $xx));CE(() ==> HH\Lib\Legacy_FIXME\eq($fx, $xx));C($fx===$xx);

  begin_row('va', 'WRAPO');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($vy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\lte($vy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\gt($vy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\gte($vy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\eq($vy, $xy));C($vy===$xy);
  begin_row('oa', 'WRAPO');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($oy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\lte($oy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\gt($oy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\gte($oy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\eq($oy, $xy));C($oy===$xy);
  begin_row('fa', 'WRAPO');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($fy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\lte($fy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\gt($fy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\gte($fy, $xy));CE(() ==> HH\Lib\Legacy_FIXME\eq($fy, $xy));C($fy===$xy);

  begin_row('va', 'WRAPD');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($vz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\lte($vz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\gt($vz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\gte($vz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\eq($vz, $xz));C($vz===$xz);
  begin_row('oa', 'WRAPD');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($oz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\lte($oz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\gt($oz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\gte($oz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\eq($oz, $xz));C($oz===$xz);
  begin_row('fa', 'WRAPD');
    CE(() ==> HH\Lib\Legacy_FIXME\lt($fz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\lte($fz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\gt($fz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\gte($fz, $xz));CE(() ==> HH\Lib\Legacy_FIXME\eq($fz, $xz));C($fz===$xz);
  print_footer();
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(handle_error<>);

  static_compare();
  dynamic_compare();
}
