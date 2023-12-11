<?hh

class foobar {}
function foobar() :mixed{}
class StrObj {
  public function __construct(private string $s)[] {}
  public function __toString(): string { return $this->s; }
}
class Wrapper { public function __construct(private mixed $w)[] {} }

function bar() :mixed{}

function LV($x)  :mixed{ return __hhvm_intrinsics\launder_value($x); }
function CLS($c) :mixed{ return __hhvm_intrinsics\create_class_pointer($c); }

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
<<__NEVER_INLINE>> function C($v) :mixed{
  try {
    printf(" %s    |", $v() ? 'T' : 'F');
  } catch (Exception $ex) {
    $m = \HH\Lib\Regex\matches(
      $ex->getMessage(),
      re"/Cannot compare .* and .* using a relational operator/",
    );
    printf(" %s    |", $m ? "E" : "X");
  }
}
<<__NEVER_INLINE>> function print_footer() :mixed{
  echo "\n+------------+------+------+------+------+------+------+\n\n";
}

<<__NEVER_INLINE>> function static_compare() :mixed{
  $cm = foobar<>;
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
    C(()==>$cm<$va);C(()==>$cm<=$va);C(()==>$cm>$va);C(()==>$cm>=$va);C(()==>$cm==$va);C(()==>$cm===$va);
  begin_row('oa');
    C(()==>$cm<$oa);C(()==>$cm<=$oa);C(()==>$cm>$oa);C(()==>$cm>=$oa);C(()==>$cm==$oa);C(()==>$cm===$oa);
  begin_row('fa');
    C(()==>$cm<$fa);C(()==>$cm<=$fa);C(()==>$cm>$fa);C(()==>$cm>=$fa);C(()==>$cm==$fa);C(()==>$cm===$fa);

  begin_row('va', 'WRAPA');
    C(()==>$xx<$vx);C(()==>$xx<=$vx);C(()==>$xx>$vx);C(()==>$xx>=$vx);C(()==>$xx==$vx);C(()==>$xx===$vx);
  begin_row('oa', 'WRAPA');
    C(()==>$xx<$ox);C(()==>$xx<=$ox);C(()==>$xx>$ox);C(()==>$xx>=$ox);C(()==>$xx==$ox);C(()==>$xx===$ox);
  begin_row('fa', 'WRAPA');
    C(()==>$xx<$fx);C(()==>$xx<=$fx);C(()==>$xx>$fx);C(()==>$xx>=$fx);C(()==>$xx==$fx);C(()==>$xx===$fx);

  begin_row('va', 'WRAPO');
    C(()==>$xy<$vy);C(()==>$xy<=$vy);C(()==>$xy>$vy);C(()==>$xy>=$vy);C(()==>$xy==$vy);C(()==>$xy===$vy);
  begin_row('oa', 'WRAPO');
    C(()==>$xy<$oy);C(()==>$xy<=$oy);C(()==>$xy>$oy);C(()==>$xy>=$oy);C(()==>$xy==$oy);C(()==>$xy===$oy);
  begin_row('fa', 'WRAPO');
    C(()==>$xy<$fy);C(()==>$xy<=$fy);C(()==>$xy>$fy);C(()==>$xy>=$fy);C(()==>$xy==$fy);C(()==>$xy===$fy);

  begin_row('va', 'WRAPD');
    C(()==>$xz<$vz);C(()==>$xz<=$vz);C(()==>$xz>$vz);C(()==>$xz>=$vz);C(()==>$xz==$vz);C(()==>$xz===$vz);
  begin_row('oa', 'WRAPD');
    C(()==>$xz<$oz);C(()==>$xz<=$oz);C(()==>$xz>$oz);C(()==>$xz>=$oz);C(()==>$xz==$oz);C(()==>$xz===$oz);
  begin_row('fa', 'WRAPD');
    C(()==>$xz<$fz);C(()==>$xz<=$fz);C(()==>$xz>$fz);C(()==>$xz>=$fz);C(()==>$xz==$fz);C(()==>$xz===$fz);
  print_footer();

  print_header('[static] VAR ? $cm');
  begin_row('va');
    C(()==>$va<$cm);C(()==>$va<=$cm);C(()==>$va>$cm);C(()==>$va>=$cm);C(()==>$va==$cm);C(()==>$va===$cm);
  begin_row('oa');
    C(()==>$oa<$cm);C(()==>$oa<=$cm);C(()==>$oa>$cm);C(()==>$oa>=$cm);C(()==>$oa==$cm);C(()==>$oa===$cm);
  begin_row('fa');
    C(()==>$fa<$cm);C(()==>$fa<=$cm);C(()==>$fa>$cm);C(()==>$fa>=$cm);C(()==>$fa==$cm);C(()==>$fa===$cm);

  begin_row('va', 'WRAPA');
    C(()==>$vx<$xx);C(()==>$vx<=$xx);C(()==>$vx>$xx);C(()==>$vx>=$xx);C(()==>$vx==$xx);C(()==>$vx===$xx);
  begin_row('oa', 'WRAPA');
    C(()==>$ox<$xx);C(()==>$ox<=$xx);C(()==>$ox>$xx);C(()==>$ox>=$xx);C(()==>$ox==$xx);C(()==>$ox===$xx);
  begin_row('fa', 'WRAPA');
    C(()==>$fx<$xx);C(()==>$fx<=$xx);C(()==>$fx>$xx);C(()==>$fx>=$xx);C(()==>$fx==$xx);C(()==>$fx===$xx);

  begin_row('va', 'WRAPO');
    C(()==>$vy<$xy);C(()==>$vy<=$xy);C(()==>$vy>$xy);C(()==>$vy>=$xy);C(()==>$vy==$xy);C(()==>$vy===$xy);
  begin_row('oa', 'WRAPO');
    C(()==>$oy<$xy);C(()==>$oy<=$xy);C(()==>$oy>$xy);C(()==>$oy>=$xy);C(()==>$oy==$xy);C(()==>$oy===$xy);
  begin_row('fa', 'WRAPO');
    C(()==>$fy<$xy);C(()==>$fy<=$xy);C(()==>$fy>$xy);C(()==>$fy>=$xy);C(()==>$fy==$xy);C(()==>$fy===$xy);

  begin_row('va', 'WRAPD');
    C(()==>$vz<$xz);C(()==>$vz<=$xz);C(()==>$vz>$xz);C(()==>$vz>=$xz);C(()==>$vz==$xz);C(()==>$vz===$xz);
  begin_row('oa', 'WRAPD');
    C(()==>$oz<$xz);C(()==>$oz<=$xz);C(()==>$oz>$xz);C(()==>$oz>=$xz);C(()==>$oz==$xz);C(()==>$oz===$xz);
  begin_row('fa', 'WRAPD');
    C(()==>$fz<$xz);C(()==>$fz<=$xz);C(()==>$fz>$xz);C(()==>$fz>=$xz);C(()==>$fz==$xz);C(()==>$fz===$xz);
  print_footer();
}

<<__NEVER_INLINE>> function dynamic_compare() :mixed{
  $cm = LV(foobar<>);
  $va = LV('foobar');
  $oa = LV(new StrObj('foobar'));
  $fa = LV(CLS('foobar'));

  $xx = WRAPA($cm); $vx = WRAPA($va); $ox = WRAPA($oa); $fx = WRAPA($fa);

  $xy = WRAPO($cm); $vy = WRAPO($va); $oy = WRAPO($oa); $fy = WRAPO($fa);

  $xz = WRAPD($cm); $vz = WRAPD($va); $oz = WRAPD($oa); $fz = WRAPD($fa);

  print_header('[dynamic] $cm ? VAR');
  begin_row('va');
    C(()==>$cm<$va);C(()==>$cm<=$va);C(()==>$cm>$va);C(()==>$cm>=$va);C(()==>$cm==$va);C(()==>$cm===$va);
  begin_row('oa');
    C(()==>$cm<$oa);C(()==>$cm<=$oa);C(()==>$cm>$oa);C(()==>$cm>=$oa);C(()==>$cm==$oa);C(()==>$cm===$oa);
  begin_row('fa');
    C(()==>$cm<$fa);C(()==>$cm<=$fa);C(()==>$cm>$fa);C(()==>$cm>=$fa);C(()==>$cm==$fa);C(()==>$cm===$fa);

  begin_row('va', 'WRAPA');
    C(()==>$xx<$vx);C(()==>$xx<=$vx);C(()==>$xx>$vx);C(()==>$xx>=$vx);C(()==>$xx==$vx);C(()==>$xx===$vx);
  begin_row('oa', 'WRAPA');
    C(()==>$xx<$ox);C(()==>$xx<=$ox);C(()==>$xx>$ox);C(()==>$xx>=$ox);C(()==>$xx==$ox);C(()==>$xx===$ox);
  begin_row('fa', 'WRAPA');
    C(()==>$xx<$fx);C(()==>$xx<=$fx);C(()==>$xx>$fx);C(()==>$xx>=$fx);C(()==>$xx==$fx);C(()==>$xx===$fx);

  begin_row('va', 'WRAPO');
    C(()==>$xy<$vy);C(()==>$xy<=$vy);C(()==>$xy>$vy);C(()==>$xy>=$vy);C(()==>$xy==$vy);C(()==>$xy===$vy);
  begin_row('oa', 'WRAPO');
    C(()==>$xy<$oy);C(()==>$xy<=$oy);C(()==>$xy>$oy);C(()==>$xy>=$oy);C(()==>$xy==$oy);C(()==>$xy===$oy);
  begin_row('fa', 'WRAPO');
    C(()==>$xy<$fy);C(()==>$xy<=$fy);C(()==>$xy>$fy);C(()==>$xy>=$fy);C(()==>$xy==$fy);C(()==>$xy===$fy);

  begin_row('va', 'WRAPD');
    C(()==>$xz<$vz);C(()==>$xz<=$vz);C(()==>$xz>$vz);C(()==>$xz>=$vz);C(()==>$xz==$vz);C(()==>$xz===$vz);
  begin_row('oa', 'WRAPD');
    C(()==>$xz<$oz);C(()==>$xz<=$oz);C(()==>$xz>$oz);C(()==>$xz>=$oz);C(()==>$xz==$oz);C(()==>$xz===$oz);
  begin_row('fa', 'WRAPD');
    C(()==>$xz<$fz);C(()==>$xz<=$fz);C(()==>$xz>$fz);C(()==>$xz>=$fz);C(()==>$xz==$fz);C(()==>$xz===$fz);
  print_footer();

  print_header('[dynamic] VAR ? $cm');
  begin_row('va');
    C(()==>$va<$cm);C(()==>$va<=$cm);C(()==>$va>$cm);C(()==>$va>=$cm);C(()==>$va==$cm);C(()==>$va===$cm);
  begin_row('oa');
    C(()==>$oa<$cm);C(()==>$oa<=$cm);C(()==>$oa>$cm);C(()==>$oa>=$cm);C(()==>$oa==$cm);C(()==>$oa===$cm);
  begin_row('fa');
    C(()==>$fa<$cm);C(()==>$fa<=$cm);C(()==>$fa>$cm);C(()==>$fa>=$cm);C(()==>$fa==$cm);C(()==>$fa===$cm);

  begin_row('va', 'WRAPA');
    C(()==>$vx<$xx);C(()==>$vx<=$xx);C(()==>$vx>$xx);C(()==>$vx>=$xx);C(()==>$vx==$xx);C(()==>$vx===$xx);
  begin_row('oa', 'WRAPA');
    C(()==>$ox<$xx);C(()==>$ox<=$xx);C(()==>$ox>$xx);C(()==>$ox>=$xx);C(()==>$ox==$xx);C(()==>$ox===$xx);
  begin_row('fa', 'WRAPA');
    C(()==>$fx<$xx);C(()==>$fx<=$xx);C(()==>$fx>$xx);C(()==>$fx>=$xx);C(()==>$fx==$xx);C(()==>$fx===$xx);

  begin_row('va', 'WRAPO');
    C(()==>$vy<$xy);C(()==>$vy<=$xy);C(()==>$vy>$xy);C(()==>$vy>=$xy);C(()==>$vy==$xy);C(()==>$vy===$xy);
  begin_row('oa', 'WRAPO');
    C(()==>$oy<$xy);C(()==>$oy<=$xy);C(()==>$oy>$xy);C(()==>$oy>=$xy);C(()==>$oy==$xy);C(()==>$oy===$xy);
  begin_row('fa', 'WRAPO');
    C(()==>$fy<$xy);C(()==>$fy<=$xy);C(()==>$fy>$xy);C(()==>$fy>=$xy);C(()==>$fy==$xy);C(()==>$fy===$xy);

  begin_row('va', 'WRAPD');
    C(()==>$vz<$xz);C(()==>$vz<=$xz);C(()==>$vz>$xz);C(()==>$vz>=$xz);C(()==>$vz==$xz);C(()==>$vz===$xz);
  begin_row('oa', 'WRAPD');
    C(()==>$oz<$xz);C(()==>$oz<=$xz);C(()==>$oz>$xz);C(()==>$oz>=$xz);C(()==>$oz==$xz);C(()==>$oz===$xz);
  begin_row('fa', 'WRAPD');
    C(()==>$fz<$xz);C(()==>$fz<=$xz);C(()==>$fz>$xz);C(()==>$fz>=$xz);C(()==>$fz==$xz);C(()==>$fz===$xz);
  print_footer();
}

<<__EntryPoint>>
function main() :mixed{
  static_compare();
  dynamic_compare();
}
