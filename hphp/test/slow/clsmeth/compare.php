<?hh

class Foo { static function bar() {} }
class StrObj {
  public function __construct(private string $s) {}
  public function __toString(): string { return $this->s; }
}
class Wrapper { public function __construct(private mixed $w) {} }

function bar() {}

function LV($x)  { return __hhvm_intrinsics\launder_value($x); }
function CLS($c) { return __hhvm_intrinsics\create_class_pointer($c); }

function WRAPA($x) { return LV(varray[$x]); }
function WRAPO($x) { return LV(new Wrapper($x)); }
function WRAPD($x) { $r = new stdClass; $r->x = $x; return LV($r); }

<<__NEVER_INLINE>> function print_header($title) {
  echo "$title\n";
  echo "+------------+------+------+------+------+------+------+------+\n";
  echo "| VAR        | <    | <=   | >    | >=   | ==   | ===  | <=>  |\n";
  echo "+============+======+======+======+======+======+======+======+";
}
<<__NEVER_INLINE>> function begin_row($var, $wrap = null) {
  printf("\n| %-10s |", $wrap !== null ? $wrap."(\$$var)" : "\$$var");
}
<<__NEVER_INLINE>> function C(bool $v) {
  printf(" %s    |", $v ? 'T' : 'F');
}
<<__NEVER_INLINE>> function Cx($f) {
  try {
    C($f());
  } catch (InvalidOperationException $e) {
    print(" EXN  |");
  }
}
<<__NEVER_INLINE>> function print_footer() {
  echo "\n+------------+------+------+------+------+------+------+------+\n\n";
}
<<__NEVER_INLINE>> function I(int $v) {
  printf(" %-4d |", $v);
}
<<__NEVER_INLINE>> function Ix($f) {
  try {
    I($f());
  } catch (InvalidOperationException $e) {
    print(" EXN  |");
  }
}

<<__NEVER_INLINE>> function static_compare() {
  $cm = class_meth(Foo::class, 'bar');
  $va = varray[Foo::class, 'bar'];
  $oa = varray[new StrObj(Foo::class), new StrObj('bar')];
  $fa = varray[Foo::class, bar<>];
  $ca = varray[CLS('Foo'), 'bar'];
  $pa = varray[CLS('Foo'), bar<>];

  $xx = varray[$cm]; $vx = varray[$va]; $ox = varray[$oa]; $fx = varray[$fa];
  $cx = varray[$ca]; $px = varray[$pa];

  $xy = new Wrapper($cm); $vy = new Wrapper($va); $oy = new Wrapper($oa);
  $fy = new Wrapper($fa); $cy = new Wrapper($ca); $py = new Wrapper($pa);

  $xz = new stdClass; $xz->v = $cm; $vz = new stdClass; $vz->v = $va;
  $oz = new stdClass; $oz->v = $oa; $fz = new stdClass; $fz->v = $fa;
  $cz = new stdClass; $cz->v = $ca; $pz = new stdClass; $pz->v = $pa;

  print_header('[static] $cm ? VAR');
  begin_row('va');
    C($cm<$va);C($cm<=$va);C($cm>$va);C($cm>=$va);C($cm==$va);C($cm===$va);
    I($cm<=>$va);
  begin_row('oa');
    C($cm<$oa);C($cm<=$oa);C($cm>$oa);C($cm>=$oa);C($cm==$oa);C($cm===$oa);
    I($cm<=>$oa);
  begin_row('fa');
    Cx(() ==> $cm<$fa);Cx(() ==> $cm<=$fa);Cx(() ==> $cm>$fa);Cx(() ==> $cm>=$fa);Cx(() ==> $cm==$fa);Cx(() ==> $cm===$fa);
    Ix(() ==> $cm<=>$fa);
  begin_row('ca');
    C($cm<$ca);C($cm<=$ca);C($cm>$ca);C($cm>=$ca);C($cm==$ca);C($cm===$ca);
    I($cm<=>$ca);
  begin_row('pa');
    Cx(() ==> $cm<$pa);Cx(() ==> $cm<=$pa);Cx(() ==> $cm>$pa);Cx(() ==> $cm>=$pa);Cx(() ==> $cm==$pa);Cx(() ==> $cm===$pa);
    Ix(() ==> $cm<=>$pa);

  begin_row('va', 'WRAPA');
    C($xx<$vx);C($xx<=$vx);C($xx>$vx);C($xx>=$vx);C($xx==$vx);C($xx===$vx);
    I($xx<=>$vx);
  begin_row('oa', 'WRAPA');
    C($xx<$ox);C($xx<=$ox);C($xx>$ox);C($xx>=$ox);C($xx==$ox);C($xx===$ox);
    I($xx<=>$ox);
  begin_row('fa', 'WRAPA');
    Cx(() ==> $xx<$fx);Cx(() ==> $xx<=$fx);Cx(() ==> $xx>$fx);Cx(() ==> $xx>=$fx);Cx(() ==> $xx==$fx);Cx(() ==> $xx===$fx);
    Ix(() ==> $xx<=>$fx);
  begin_row('ca', 'WRAPA');
    C($xx<$cx);C($xx<=$cx);C($xx>$cx);C($xx>=$cx);C($xx==$cx);C($xx===$cx);
    I($xx<=>$cx);
  begin_row('pa', 'WRAPA');
    Cx(() ==> $xx<$px);Cx(() ==> $xx<=$px);Cx(() ==> $xx>$px);Cx(() ==> $xx>=$px);Cx(() ==> $xx==$px);Cx(() ==> $xx===$px);
    Ix(() ==> $xx<=>$px);

  begin_row('va', 'WRAPO');
    C($xy<$vy);C($xy<=$vy);C($xy>$vy);C($xy>=$vy);C($xy==$vy);C($xy===$vy);
    I($xy<=>$vy);
  begin_row('oa', 'WRAPO');
    C($xy<$oy);C($xy<=$oy);C($xy>$oy);C($xy>=$oy);C($xy==$oy);C($xy===$oy);
    I($xy<=>$oy);
  begin_row('fa', 'WRAPO');
    Cx(() ==> $xy<$fy);Cx(() ==> $xy<=$fy);Cx(() ==> $xy>$fy);Cx(() ==> $xy>=$fy);Cx(() ==> $xy==$fy);Cx(() ==> $xy===$fy);
    Ix(() ==> $xy<=>$fy);
  begin_row('ca', 'WRAPO');
    C($xy<$cy);C($xy<=$cy);C($xy>$cy);C($xy>=$cy);C($xy==$cy);C($xy===$cy);
    I($xy<=>$cy);
  begin_row('pa', 'WRAPO');
    Cx(() ==> $xy<$py);Cx(() ==> $xy<=$py);Cx(() ==> $xy>$py);Cx(() ==> $xy>=$py);Cx(() ==> $xy==$py);Cx(() ==> $xy===$py);
    Ix(() ==> $xy<=>$py);

  begin_row('va', 'WRAPD');
    C($xz<$vz);C($xz<=$vz);C($xz>$vz);C($xz>=$vz);C($xz==$vz);C($xz===$vz);
    I($xz<=>$vz);
  begin_row('oa', 'WRAPD');
    C($xz<$oz);C($xz<=$oz);C($xz>$oz);C($xz>=$oz);C($xz==$oz);C($xz===$oz);
    I($xz<=>$oz);
  begin_row('fa', 'WRAPD');
    Cx(() ==> $xz<$fz);Cx(() ==> $xz<=$fz);Cx(() ==> $xz>$fz);Cx(() ==> $xz>=$fz);Cx(() ==> $xz==$fz);Cx(() ==> $xz===$fz);
    Ix(() ==> $xz<=>$fz);
  begin_row('ca', 'WRAPD');
    C($xz<$cz);C($xz<=$cz);C($xz>$cz);C($xz>=$cz);C($xz==$cz);C($xz===$cz);
    I($xz<=>$cz);
  begin_row('pa', 'WRAPD');
    Cx(() ==> $xz<$pz);Cx(() ==> $xz<=$pz);Cx(() ==> $xz>$pz);Cx(() ==> $xz>=$pz);Cx(() ==> $xz==$pz);Cx(() ==> $xz===$pz);
    Ix(() ==> $xz<=>$pz);
  print_footer();

  print_header('[static] VAR ? $cm');
  begin_row('va');
    C($va<$cm);C($va<=$cm);C($va>$cm);C($va>=$cm);C($va==$cm);C($va===$cm);
    I($va<=>$cm);
  begin_row('oa');
    C($oa<$cm);C($oa<=$cm);C($oa>$cm);C($oa>=$cm);C($oa==$cm);C($oa===$cm);
    I($oa<=>$cm);
  begin_row('fa');
    Cx(() ==> $fa<$cm);Cx(() ==> $fa<=$cm);Cx(() ==> $fa>$cm);Cx(() ==> $fa>=$cm);Cx(() ==> $fa==$cm);Cx(() ==> $fa===$cm);
    Ix(() ==> $fa<=>$cm);
  begin_row('ca');
    C($ca<$cm);C($ca<=$cm);C($ca>$cm);C($ca>=$cm);C($ca==$cm);C($ca===$cm);
    I($ca<=>$cm);
  begin_row('pa');
    Cx(() ==> $pa<$cm);Cx(() ==> $pa<=$cm);Cx(() ==> $pa>$cm);Cx(() ==> $pa>=$cm);Cx(() ==> $pa==$cm);Cx(() ==> $pa===$cm);
    Ix(() ==> $pa<=>$cm);

  begin_row('va', 'WRAPA');
    C($vx<$xx);C($vx<=$xx);C($vx>$xx);C($vx>=$xx);C($vx==$xx);C($vx===$xx);
    I($vx<=>$xx);
  begin_row('oa', 'WRAPA');
    C($ox<$xx);C($ox<=$xx);C($ox>$xx);C($ox>=$xx);C($ox==$xx);C($ox===$xx);
    I($ox<=>$xx);
  begin_row('fa', 'WRAPA');
    Cx(() ==> $fx<$xx);Cx(() ==> $fx<=$xx);Cx(() ==> $fx>$xx);Cx(() ==> $fx>=$xx);Cx(() ==> $fx==$xx);Cx(() ==> $fx===$xx);
    Ix(() ==> $fx<=>$xx);
  begin_row('ca', 'WRAPA');
    C($cx<$xx);C($cx<=$xx);C($cx>$xx);C($cx>=$xx);C($cx==$xx);C($cx===$xx);
    I($cx<=>$xx);
  begin_row('pa', 'WRAPA');
    Cx(() ==> $px<$xx);Cx(() ==> $px<=$xx);Cx(() ==> $px>$xx);Cx(() ==> $px>=$xx);Cx(() ==> $px==$xx);Cx(() ==> $px===$xx);
    Ix(() ==> $px<=>$xx);

  begin_row('va', 'WRAPO');
    C($vy<$xy);C($vy<=$xy);C($vy>$xy);C($vy>=$xy);C($vy==$xy);C($vy===$xy);
    I($vy<=>$xy);
  begin_row('oa', 'WRAPO');
    C($oy<$xy);C($oy<=$xy);C($oy>$xy);C($oy>=$xy);C($oy==$xy);C($oy===$xy);
    I($oy<=>$xy);
  begin_row('fa', 'WRAPO');
    Cx(() ==> $fy<$xy);Cx(() ==> $fy<=$xy);Cx(() ==> $fy>$xy);Cx(() ==> $fy>=$xy);Cx(() ==> $fy==$xy);Cx(() ==> $fy===$xy);
    Ix(() ==> $fy<=>$xy);
  begin_row('ca', 'WRAPO');
    C($cy<$xy);C($cy<=$xy);C($cy>$xy);C($cy>=$xy);C($cy==$xy);C($cy===$xy);
    I($cy<=>$xy);
  begin_row('pa', 'WRAPO');
    Cx(() ==> $py<$xy);Cx(() ==> $py<=$xy);Cx(() ==> $py>$xy);Cx(() ==> $py>=$xy);Cx(() ==> $py==$xy);Cx(() ==> $py===$xy);
    Ix(() ==> $py<=>$xy);

  begin_row('va', 'WRAPD');
    C($vz<$xz);C($vz<=$xz);C($vz>$xz);C($vz>=$xz);C($vz==$xz);C($vz===$xz);
    I($vz<=>$xz);
  begin_row('oa', 'WRAPD');
    C($oz<$xz);C($oz<=$xz);C($oz>$xz);C($oz>=$xz);C($oz==$xz);C($oz===$xz);
    I($oz<=>$xz);
  begin_row('fa', 'WRAPD');
    Cx(() ==> $fz<$xz);Cx(() ==> $fz<=$xz);Cx(() ==> $fz>$xz);Cx(() ==> $fz>=$xz);Cx(() ==> $fz==$xz);Cx(() ==> $fz===$xz);
    Ix(() ==> $fz<=>$xz);
  begin_row('ca', 'WRAPD');
    C($cz<$xz);C($cz<=$xz);C($cz>$xz);C($cz>=$xz);C($cz==$xz);C($cz===$xz);
    I($cz<=>$xz);
  begin_row('pa', 'WRAPD');
    Cx(() ==> $pz<$xz);Cx(() ==> $pz<=$xz);Cx(() ==> $pz>$xz);Cx(() ==> $pz>=$xz);Cx(() ==> $pz==$xz);Cx(() ==> $pz===$xz);
    Ix(() ==> $pz<=>$xz);
  print_footer();
}

<<__NEVER_INLINE>> function dynamic_compare() {
  $cm = LV(class_meth(Foo::class, 'bar'));
  $va = LV(varray[Foo::class, 'bar']);
  $oa = LV(varray[new StrObj(Foo::class), new StrObj('bar')]);
  $fa = LV(varray[Foo::class, bar<>]);
  $ca = LV(varray[CLS('Foo'), 'bar']);
  $pa = LV(varray[CLS('Foo'), bar<>]);

  $xx = WRAPA($cm); $vx = WRAPA($va); $ox = WRAPA($oa); $fx = WRAPA($fa);
  $cx = WRAPA($ca); $px = WRAPA($pa);

  $xy = WRAPO($cm); $vy = WRAPO($va); $oy = WRAPO($oa); $fy = WRAPO($fa);
  $cy = WRAPO($ca); $py = WRAPO($pa);

  $xz = WRAPD($cm); $vz = WRAPD($va); $oz = WRAPD($oa); $fz = WRAPD($fa);
  $cz = WRAPD($ca); $pz = WRAPD($pa);

  print_header('[dynamic] $cm ? VAR');
  begin_row('va');
    C($cm<$va);C($cm<=$va);C($cm>$va);C($cm>=$va);C($cm==$va);C($cm===$va);
    I($cm<=>$va);
  begin_row('oa');
    C($cm<$oa);C($cm<=$oa);C($cm>$oa);C($cm>=$oa);C($cm==$oa);C($cm===$oa);
    I($cm<=>$oa);
  begin_row('fa');
    Cx(() ==> $cm<$fa);Cx(() ==> $cm<=$fa);Cx(() ==> $cm>$fa);Cx(() ==> $cm>=$fa);Cx(() ==> $cm==$fa);Cx(() ==> $cm===$fa);
    Ix(() ==> $cm<=>$fa);
  begin_row('ca');
    C($cm<$ca);C($cm<=$ca);C($cm>$ca);C($cm>=$ca);C($cm==$ca);C($cm===$ca);
    I($cm<=>$ca);
  begin_row('pa');
    Cx(() ==> $cm<$pa);Cx(() ==> $cm<=$pa);Cx(() ==> $cm>$pa);Cx(() ==> $cm>=$pa);Cx(() ==> $cm==$pa);Cx(() ==> $cm===$pa);
    Ix(() ==> $cm<=>$pa);

  begin_row('va', 'WRAPA');
    C($xx<$vx);C($xx<=$vx);C($xx>$vx);C($xx>=$vx);C($xx==$vx);C($xx===$vx);
    I($xx<=>$vx);
  begin_row('oa', 'WRAPA');
    C($xx<$ox);C($xx<=$ox);C($xx>$ox);C($xx>=$ox);C($xx==$ox);C($xx===$ox);
    I($xx<=>$ox);
  begin_row('fa', 'WRAPA');
    Cx(() ==> $xx<$fx);Cx(() ==> $xx<=$fx);Cx(() ==> $xx>$fx);Cx(() ==> $xx>=$fx);Cx(() ==> $xx==$fx);Cx(() ==> $xx===$fx);
    Ix(() ==> $xx<=>$fx);
  begin_row('ca', 'WRAPA');
    C($xx<$cx);C($xx<=$cx);C($xx>$cx);C($xx>=$cx);C($xx==$cx);C($xx===$cx);
    I($xx<=>$cx);
  begin_row('pa', 'WRAPA');
    Cx(() ==> $xx<$px);Cx(() ==> $xx<=$px);Cx(() ==> $xx>$px);Cx(() ==> $xx>=$px);Cx(() ==> $xx==$px);Cx(() ==> $xx===$px);
    Ix(() ==> $xx<=>$px);

  begin_row('va', 'WRAPO');
    C($xy<$vy);C($xy<=$vy);C($xy>$vy);C($xy>=$vy);C($xy==$vy);C($xy===$vy);
    I($xy<=>$vy);
  begin_row('oa', 'WRAPO');
    C($xy<$oy);C($xy<=$oy);C($xy>$oy);C($xy>=$oy);C($xy==$oy);C($xy===$oy);
    I($xy<=>$oy);
  begin_row('fa', 'WRAPO');
    Cx(() ==> $xy<$fy);Cx(() ==> $xy<=$fy);Cx(() ==> $xy>$fy);Cx(() ==> $xy>=$fy);Cx(() ==> $xy==$fy);Cx(() ==> $xy===$fy);
    Ix(() ==> $xy<=>$fy);
  begin_row('ca', 'WRAPO');
    C($xy<$cy);C($xy<=$cy);C($xy>$cy);C($xy>=$cy);C($xy==$cy);C($xy===$cy);
    I($xy<=>$cy);
  begin_row('pa', 'WRAPO');
    Cx(() ==> $xy<$py);Cx(() ==> $xy<=$py);Cx(() ==> $xy>$py);Cx(() ==> $xy>=$py);Cx(() ==> $xy==$py);Cx(() ==> $xy===$py);
    Ix(() ==> $xy<=>$py);

  begin_row('va', 'WRAPD');
    C($xz<$vz);C($xz<=$vz);C($xz>$vz);C($xz>=$vz);C($xz==$vz);C($xz===$vz);
    I($xz<=>$vz);
  begin_row('oa', 'WRAPD');
    C($xz<$oz);C($xz<=$oz);C($xz>$oz);C($xz>=$oz);C($xz==$oz);C($xz===$oz);
    I($xz<=>$oz);
  begin_row('fa', 'WRAPD');
    Cx(() ==> $xz<$fz);Cx(() ==> $xz<=$fz);Cx(() ==> $xz>$fz);Cx(() ==> $xz>=$fz);Cx(() ==> $xz==$fz);Cx(() ==> $xz===$fz);
    Ix(() ==> $xz<=>$fz);
  begin_row('ca', 'WRAPD');
    C($xz<$cz);C($xz<=$cz);C($xz>$cz);C($xz>=$cz);C($xz==$cz);C($xz===$cz);
    I($xz<=>$cz);
  begin_row('pa', 'WRAPD');
    Cx(() ==> $xz<$pz);Cx(() ==> $xz<=$pz);Cx(() ==> $xz>$pz);Cx(() ==> $xz>=$pz);Cx(() ==> $xz==$pz);Cx(() ==> $xz===$pz);
    Ix(() ==> $xz<=>$pz);
  print_footer();

  print_header('[dynamic] VAR ? $cm');
  begin_row('va');
    C($va<$cm);C($va<=$cm);C($va>$cm);C($va>=$cm);C($va==$cm);C($va===$cm);
    I($va<=>$cm);
  begin_row('oa');
    C($oa<$cm);C($oa<=$cm);C($oa>$cm);C($oa>=$cm);C($oa==$cm);C($oa===$cm);
    I($oa<=>$cm);
  begin_row('fa');
    Cx(() ==> $fa<$cm);Cx(() ==> $fa<=$cm);Cx(() ==> $fa>$cm);Cx(() ==> $fa>=$cm);Cx(() ==> $fa==$cm);Cx(() ==> $fa===$cm);
    Ix(() ==> $fa<=>$cm);
  begin_row('ca');
    C($ca<$cm);C($ca<=$cm);C($ca>$cm);C($ca>=$cm);C($ca==$cm);C($ca===$cm);
    I($ca<=>$cm);
  begin_row('pa');
    Cx(() ==> $pa<$cm);Cx(() ==> $pa<=$cm);Cx(() ==> $pa>$cm);Cx(() ==> $pa>=$cm);Cx(() ==> $pa==$cm);Cx(() ==> $pa===$cm);
    Ix(() ==> $pa<=>$cm);

  begin_row('va', 'WRAPA');
    C($vx<$xx);C($vx<=$xx);C($vx>$xx);C($vx>=$xx);C($vx==$xx);C($vx===$xx);
    I($vx<=>$xx);
  begin_row('oa', 'WRAPA');
    C($ox<$xx);C($ox<=$xx);C($ox>$xx);C($ox>=$xx);C($ox==$xx);C($ox===$xx);
    I($ox<=>$xx);
  begin_row('fa', 'WRAPA');
    Cx(() ==> $fx<$xx);Cx(() ==> $fx<=$xx);Cx(() ==> $fx>$xx);Cx(() ==> $fx>=$xx);Cx(() ==> $fx==$xx);Cx(() ==> $fx===$xx);
    Ix(() ==> $fx<=>$xx);
  begin_row('ca', 'WRAPA');
    C($cx<$xx);C($cx<=$xx);C($cx>$xx);C($cx>=$xx);C($cx==$xx);C($cx===$xx);
    I($cx<=>$xx);
  begin_row('pa', 'WRAPA');
    Cx(() ==> $px<$xx);Cx(() ==> $px<=$xx);Cx(() ==> $px>$xx);Cx(() ==> $px>=$xx);Cx(() ==> $px==$xx);Cx(() ==> $px===$xx);
    Ix(() ==> $px<=>$xx);

  begin_row('va', 'WRAPO');
    C($vy<$xy);C($vy<=$xy);C($vy>$xy);C($vy>=$xy);C($vy==$xy);C($vy===$xy);
    I($vy<=>$xy);
  begin_row('oa', 'WRAPO');
    C($oy<$xy);C($oy<=$xy);C($oy>$xy);C($oy>=$xy);C($oy==$xy);C($oy===$xy);
    I($oy<=>$xy);
  begin_row('fa', 'WRAPO');
    Cx(() ==> $fy<$xy);Cx(() ==> $fy<=$xy);Cx(() ==> $fy>$xy);Cx(() ==> $fy>=$xy);Cx(() ==> $fy==$xy);Cx(() ==> $fy===$xy);
    Ix(() ==> $fy<=>$xy);
  begin_row('ca', 'WRAPO');
    C($cy<$xy);C($cy<=$xy);C($cy>$xy);C($cy>=$xy);C($cy==$xy);C($cy===$xy);
    I($cy<=>$xy);
  begin_row('pa', 'WRAPO');
    Cx(() ==> $py<$xy);Cx(() ==> $py<=$xy);Cx(() ==> $py>$xy);Cx(() ==> $py>=$xy);Cx(() ==> $py==$xy);Cx(() ==> $py===$xy);
    Ix(() ==> $py<=>$xy);

  begin_row('va', 'WRAPD');
    C($vz<$xz);C($vz<=$xz);C($vz>$xz);C($vz>=$xz);C($vz==$xz);C($vz===$xz);
    I($vz<=>$xz);
  begin_row('oa', 'WRAPD');
    C($oz<$xz);C($oz<=$xz);C($oz>$xz);C($oz>=$xz);C($oz==$xz);C($oz===$xz);
    I($oz<=>$xz);
  begin_row('fa', 'WRAPD');
    Cx(() ==> $fz<$xz);Cx(() ==> $fz<=$xz);Cx(() ==> $fz>$xz);Cx(() ==> $fz>=$xz);Cx(() ==> $fz==$xz);Cx(() ==> $fz===$xz);
    Ix(() ==> $fz<=>$xz);
  begin_row('ca', 'WRAPD');
    C($cz<$xz);C($cz<=$xz);C($cz>$xz);C($cz>=$xz);C($cz==$xz);C($cz===$xz);
    I($cz<=>$xz);
  begin_row('pa', 'WRAPD');
    Cx(() ==> $pz<$xz);Cx(() ==> $pz<=$xz);Cx(() ==> $pz>$xz);Cx(() ==> $pz>=$xz);Cx(() ==> $pz==$xz);Cx(() ==> $pz===$xz);
    Ix(() ==> $pz<=>$xz);
  print_footer();
}

<<__EntryPoint>>
function main() {
  static_compare();
  dynamic_compare();
}
