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
  $fa = varray[Foo::class, fun('bar')];
  $ca = varray[CLS('Foo'), 'bar'];
  $pa = varray[CLS('Foo'), fun('bar')];

  $xx = varray[$cm]; $vx = varray[$va]; $ox = varray[$oa]; $fx = varray[$fa];
  $cx = varray[$ca]; $px = varray[$pa];

  $xy = new Wrapper($cm); $vy = new Wrapper($va); $oy = new Wrapper($oa);
  $fy = new Wrapper($fa); $cy = new Wrapper($ca); $py = new Wrapper($pa);

  $xz = new stdClass; $xz->v = $cm; $vz = new stdClass; $vz->v = $va;
  $oz = new stdClass; $oz->v = $oa; $fz = new stdClass; $fz->v = $fa;
  $cz = new stdClass; $cz->v = $ca; $pz = new stdClass; $pz->v = $pa;

  print_header('[static] $cm ? VAR');
  begin_row('va');
    Cx(() ==> $cm<$va);Cx(() ==> $cm<=$va);Cx(() ==> $cm>$va);Cx(() ==> $cm>=$va);Cx(() ==> $cm==$va);Cx(() ==> $cm===$va);
    Ix(() ==> $cm<=>$va);
  begin_row('oa');
    Cx(() ==> $cm<$oa);Cx(() ==> $cm<=$oa);Cx(() ==> $cm>$oa);Cx(() ==> $cm>=$oa);Cx(() ==> $cm==$oa);Cx(() ==> $cm===$oa);
    Ix(() ==> $cm<=>$oa);
  begin_row('fa');
    Cx(() ==> $cm<$fa);Cx(() ==> $cm<=$fa);Cx(() ==> $cm>$fa);Cx(() ==> $cm>=$fa);Cx(() ==> $cm==$fa);Cx(() ==> $cm===$fa);
    Ix(() ==> $cm<=>$fa);
  begin_row('ca');
    Cx(() ==> $cm<$ca);Cx(() ==> $cm<=$ca);Cx(() ==> $cm>$ca);Cx(() ==> $cm>=$ca);Cx(() ==> $cm==$ca);Cx(() ==> $cm===$ca);
    Ix(() ==> $cm<=>$ca);
  begin_row('pa');
    Cx(() ==> $cm<$pa);Cx(() ==> $cm<=$pa);Cx(() ==> $cm>$pa);Cx(() ==> $cm>=$pa);Cx(() ==> $cm==$pa);Cx(() ==> $cm===$pa);
    Ix(() ==> $cm<=>$pa);

  begin_row('va', 'WRAPA');
    Cx(() ==> $xx<$vx);Cx(() ==> $xx<=$vx);Cx(() ==> $xx>$vx);Cx(() ==> $xx>=$vx);Cx(() ==> $xx==$vx);Cx(() ==> $xx===$vx);
    Ix(() ==> $xx<=>$vx);
  begin_row('oa', 'WRAPA');
    Cx(() ==> $xx<$ox);Cx(() ==> $xx<=$ox);Cx(() ==> $xx>$ox);Cx(() ==> $xx>=$ox);Cx(() ==> $xx==$ox);Cx(() ==> $xx===$ox);
    Ix(() ==> $xx<=>$ox);
  begin_row('fa', 'WRAPA');
    Cx(() ==> $xx<$fx);Cx(() ==> $xx<=$fx);Cx(() ==> $xx>$fx);Cx(() ==> $xx>=$fx);Cx(() ==> $xx==$fx);Cx(() ==> $xx===$fx);
    Ix(() ==> $xx<=>$fx);
  begin_row('ca', 'WRAPA');
    Cx(() ==> $xx<$cx);Cx(() ==> $xx<=$cx);Cx(() ==> $xx>$cx);Cx(() ==> $xx>=$cx);Cx(() ==> $xx==$cx);Cx(() ==> $xx===$cx);
    Ix(() ==> $xx<=>$cx);
  begin_row('pa', 'WRAPA');
    Cx(() ==> $xx<$px);Cx(() ==> $xx<=$px);Cx(() ==> $xx>$px);Cx(() ==> $xx>=$px);Cx(() ==> $xx==$px);Cx(() ==> $xx===$px);
    Ix(() ==> $xx<=>$px);

  begin_row('va', 'WRAPO');
    Cx(() ==> $xy<$vy);Cx(() ==> $xy<=$vy);Cx(() ==> $xy>$vy);Cx(() ==> $xy>=$vy);Cx(() ==> $xy==$vy);Cx(() ==> $xy===$vy);
    Ix(() ==> $xy<=>$vy);
  begin_row('oa', 'WRAPO');
    Cx(() ==> $xy<$oy);Cx(() ==> $xy<=$oy);Cx(() ==> $xy>$oy);Cx(() ==> $xy>=$oy);Cx(() ==> $xy==$oy);Cx(() ==> $xy===$oy);
    Ix(() ==> $xy<=>$oy);
  begin_row('fa', 'WRAPO');
    Cx(() ==> $xy<$fy);Cx(() ==> $xy<=$fy);Cx(() ==> $xy>$fy);Cx(() ==> $xy>=$fy);Cx(() ==> $xy==$fy);Cx(() ==> $xy===$fy);
    Ix(() ==> $xy<=>$fy);
  begin_row('ca', 'WRAPO');
    Cx(() ==> $xy<$cy);Cx(() ==> $xy<=$cy);Cx(() ==> $xy>$cy);Cx(() ==> $xy>=$cy);Cx(() ==> $xy==$cy);Cx(() ==> $xy===$cy);
    Ix(() ==> $xy<=>$cy);
  begin_row('pa', 'WRAPO');
    Cx(() ==> $xy<$py);Cx(() ==> $xy<=$py);Cx(() ==> $xy>$py);Cx(() ==> $xy>=$py);Cx(() ==> $xy==$py);Cx(() ==> $xy===$py);
    Ix(() ==> $xy<=>$py);

  begin_row('va', 'WRAPD');
    Cx(() ==> $xz<$vz);Cx(() ==> $xz<=$vz);Cx(() ==> $xz>$vz);Cx(() ==> $xz>=$vz);Cx(() ==> $xz==$vz);Cx(() ==> $xz===$vz);
    Ix(() ==> $xz<=>$vz);
  begin_row('oa', 'WRAPD');
    Cx(() ==> $xz<$oz);Cx(() ==> $xz<=$oz);Cx(() ==> $xz>$oz);Cx(() ==> $xz>=$oz);Cx(() ==> $xz==$oz);Cx(() ==> $xz===$oz);
    Ix(() ==> $xz<=>$oz);
  begin_row('fa', 'WRAPD');
    Cx(() ==> $xz<$fz);Cx(() ==> $xz<=$fz);Cx(() ==> $xz>$fz);Cx(() ==> $xz>=$fz);Cx(() ==> $xz==$fz);Cx(() ==> $xz===$fz);
    Ix(() ==> $xz<=>$fz);
  begin_row('ca', 'WRAPD');
    Cx(() ==> $xz<$cz);Cx(() ==> $xz<=$cz);Cx(() ==> $xz>$cz);Cx(() ==> $xz>=$cz);Cx(() ==> $xz==$cz);Cx(() ==> $xz===$cz);
    Ix(() ==> $xz<=>$cz);
  begin_row('pa', 'WRAPD');
    Cx(() ==> $xz<$pz);Cx(() ==> $xz<=$pz);Cx(() ==> $xz>$pz);Cx(() ==> $xz>=$pz);Cx(() ==> $xz==$pz);Cx(() ==> $xz===$pz);
    Ix(() ==> $xz<=>$pz);
  print_footer();

  print_header('[static] VAR ? $cm');
  begin_row('va');
    Cx(() ==> $va<$cm);Cx(() ==> $va<=$cm);Cx(() ==> $va>$cm);Cx(() ==> $va>=$cm);Cx(() ==> $va==$cm);Cx(() ==> $va===$cm);
    Ix(() ==> $va<=>$cm);
  begin_row('oa');
    Cx(() ==> $oa<$cm);Cx(() ==> $oa<=$cm);Cx(() ==> $oa>$cm);Cx(() ==> $oa>=$cm);Cx(() ==> $oa==$cm);Cx(() ==> $oa===$cm);
    Ix(() ==> $oa<=>$cm);
  begin_row('fa');
    Cx(() ==> $fa<$cm);Cx(() ==> $fa<=$cm);Cx(() ==> $fa>$cm);Cx(() ==> $fa>=$cm);Cx(() ==> $fa==$cm);Cx(() ==> $fa===$cm);
    Ix(() ==> $fa<=>$cm);
  begin_row('ca');
    Cx(() ==> $ca<$cm);Cx(() ==> $ca<=$cm);Cx(() ==> $ca>$cm);Cx(() ==> $ca>=$cm);Cx(() ==> $ca==$cm);Cx(() ==> $ca===$cm);
    Ix(() ==> $ca<=>$cm);
  begin_row('pa');
    Cx(() ==> $pa<$cm);Cx(() ==> $pa<=$cm);Cx(() ==> $pa>$cm);Cx(() ==> $pa>=$cm);Cx(() ==> $pa==$cm);Cx(() ==> $pa===$cm);
    Ix(() ==> $pa<=>$cm);

  begin_row('va', 'WRAPA');
    Cx(() ==> $vx<$xx);Cx(() ==> $vx<=$xx);Cx(() ==> $vx>$xx);Cx(() ==> $vx>=$xx);Cx(() ==> $vx==$xx);Cx(() ==> $vx===$xx);
    Ix(() ==> $vx<=>$xx);
  begin_row('oa', 'WRAPA');
    Cx(() ==> $ox<$xx);Cx(() ==> $ox<=$xx);Cx(() ==> $ox>$xx);Cx(() ==> $ox>=$xx);Cx(() ==> $ox==$xx);Cx(() ==> $ox===$xx);
    Ix(() ==> $ox<=>$xx);
  begin_row('fa', 'WRAPA');
    Cx(() ==> $fx<$xx);Cx(() ==> $fx<=$xx);Cx(() ==> $fx>$xx);Cx(() ==> $fx>=$xx);Cx(() ==> $fx==$xx);Cx(() ==> $fx===$xx);
    Ix(() ==> $fx<=>$xx);
  begin_row('ca', 'WRAPA');
    Cx(() ==> $cx<$xx);Cx(() ==> $cx<=$xx);Cx(() ==> $cx>$xx);Cx(() ==> $cx>=$xx);Cx(() ==> $cx==$xx);Cx(() ==> $cx===$xx);
    Ix(() ==> $cx<=>$xx);
  begin_row('pa', 'WRAPA');
    Cx(() ==> $px<$xx);Cx(() ==> $px<=$xx);Cx(() ==> $px>$xx);Cx(() ==> $px>=$xx);Cx(() ==> $px==$xx);Cx(() ==> $px===$xx);
    Ix(() ==> $px<=>$xx);

  begin_row('va', 'WRAPO');
    Cx(() ==> $vy<$xy);Cx(() ==> $vy<=$xy);Cx(() ==> $vy>$xy);Cx(() ==> $vy>=$xy);Cx(() ==> $vy==$xy);Cx(() ==> $vy===$xy);
    Ix(() ==> $vy<=>$xy);
  begin_row('oa', 'WRAPO');
    Cx(() ==> $oy<$xy);Cx(() ==> $oy<=$xy);Cx(() ==> $oy>$xy);Cx(() ==> $oy>=$xy);Cx(() ==> $oy==$xy);Cx(() ==> $oy===$xy);
    Ix(() ==> $oy<=>$xy);
  begin_row('fa', 'WRAPO');
    Cx(() ==> $fy<$xy);Cx(() ==> $fy<=$xy);Cx(() ==> $fy>$xy);Cx(() ==> $fy>=$xy);Cx(() ==> $fy==$xy);Cx(() ==> $fy===$xy);
    Ix(() ==> $fy<=>$xy);
  begin_row('ca', 'WRAPO');
    Cx(() ==> $cy<$xy);Cx(() ==> $cy<=$xy);Cx(() ==> $cy>$xy);Cx(() ==> $cy>=$xy);Cx(() ==> $cy==$xy);Cx(() ==> $cy===$xy);
    Ix(() ==> $cy<=>$xy);
  begin_row('pa', 'WRAPO');
    Cx(() ==> $py<$xy);Cx(() ==> $py<=$xy);Cx(() ==> $py>$xy);Cx(() ==> $py>=$xy);Cx(() ==> $py==$xy);Cx(() ==> $py===$xy);
    Ix(() ==> $py<=>$xy);

  begin_row('va', 'WRAPD');
    Cx(() ==> $vz<$xz);Cx(() ==> $vz<=$xz);Cx(() ==> $vz>$xz);Cx(() ==> $vz>=$xz);Cx(() ==> $vz==$xz);Cx(() ==> $vz===$xz);
    Ix(() ==> $vz<=>$xz);
  begin_row('oa', 'WRAPD');
    Cx(() ==> $oz<$xz);Cx(() ==> $oz<=$xz);Cx(() ==> $oz>$xz);Cx(() ==> $oz>=$xz);Cx(() ==> $oz==$xz);Cx(() ==> $oz===$xz);
    Ix(() ==> $oz<=>$xz);
  begin_row('fa', 'WRAPD');
    Cx(() ==> $fz<$xz);Cx(() ==> $fz<=$xz);Cx(() ==> $fz>$xz);Cx(() ==> $fz>=$xz);Cx(() ==> $fz==$xz);Cx(() ==> $fz===$xz);
    Ix(() ==> $fz<=>$xz);
  begin_row('ca', 'WRAPD');
    Cx(() ==> $cz<$xz);Cx(() ==> $cz<=$xz);Cx(() ==> $cz>$xz);Cx(() ==> $cz>=$xz);Cx(() ==> $cz==$xz);Cx(() ==> $cz===$xz);
    Ix(() ==> $cz<=>$xz);
  begin_row('pa', 'WRAPD');
    Cx(() ==> $pz<$xz);Cx(() ==> $pz<=$xz);Cx(() ==> $pz>$xz);Cx(() ==> $pz>=$xz);Cx(() ==> $pz==$xz);Cx(() ==> $pz===$xz);
    Ix(() ==> $pz<=>$xz);
  print_footer();
}

<<__NEVER_INLINE>> function dynamic_compare() {
  $cm = LV(class_meth(Foo::class, 'bar'));
  $va = LV(varray[Foo::class, 'bar']);
  $oa = LV(varray[new StrObj(Foo::class), new StrObj('bar')]);
  $fa = LV(varray[Foo::class, fun('bar')]);
  $ca = LV(varray[CLS('Foo'), 'bar']);
  $pa = LV(varray[CLS('Foo'), fun('bar')]);

  $xx = WRAPA($cm); $vx = WRAPA($va); $ox = WRAPA($oa); $fx = WRAPA($fa);
  $cx = WRAPA($ca); $px = WRAPA($pa);

  $xy = WRAPO($cm); $vy = WRAPO($va); $oy = WRAPO($oa); $fy = WRAPO($fa);
  $cy = WRAPO($ca); $py = WRAPO($pa);

  $xz = WRAPD($cm); $vz = WRAPD($va); $oz = WRAPD($oa); $fz = WRAPD($fa);
  $cz = WRAPD($ca); $pz = WRAPD($pa);

  print_header('[dynamic] $cm ? VAR');
  begin_row('va');
    Cx(() ==> $cm<$va);Cx(() ==> $cm<=$va);Cx(() ==> $cm>$va);Cx(() ==> $cm>=$va);Cx(() ==> $cm==$va);Cx(() ==> $cm===$va);
    Ix(() ==> $cm<=>$va);
  begin_row('oa');
    Cx(() ==> $cm<$oa);Cx(() ==> $cm<=$oa);Cx(() ==> $cm>$oa);Cx(() ==> $cm>=$oa);Cx(() ==> $cm==$oa);Cx(() ==> $cm===$oa);
    Ix(() ==> $cm<=>$oa);
  begin_row('fa');
    Cx(() ==> $cm<$fa);Cx(() ==> $cm<=$fa);Cx(() ==> $cm>$fa);Cx(() ==> $cm>=$fa);Cx(() ==> $cm==$fa);Cx(() ==> $cm===$fa);
    Ix(() ==> $cm<=>$fa);
  begin_row('ca');
    Cx(() ==> $cm<$ca);Cx(() ==> $cm<=$ca);Cx(() ==> $cm>$ca);Cx(() ==> $cm>=$ca);Cx(() ==> $cm==$ca);Cx(() ==> $cm===$ca);
    Ix(() ==> $cm<=>$ca);
  begin_row('pa');
    Cx(() ==> $cm<$pa);Cx(() ==> $cm<=$pa);Cx(() ==> $cm>$pa);Cx(() ==> $cm>=$pa);Cx(() ==> $cm==$pa);Cx(() ==> $cm===$pa);
    Ix(() ==> $cm<=>$pa);

  begin_row('va', 'WRAPA');
    Cx(() ==> $xx<$vx);Cx(() ==> $xx<=$vx);Cx(() ==> $xx>$vx);Cx(() ==> $xx>=$vx);Cx(() ==> $xx==$vx);Cx(() ==> $xx===$vx);
    Ix(() ==> $xx<=>$vx);
  begin_row('oa', 'WRAPA');
    Cx(() ==> $xx<$ox);Cx(() ==> $xx<=$ox);Cx(() ==> $xx>$ox);Cx(() ==> $xx>=$ox);Cx(() ==> $xx==$ox);Cx(() ==> $xx===$ox);
    Ix(() ==> $xx<=>$ox);
  begin_row('fa', 'WRAPA');
    Cx(() ==> $xx<$fx);Cx(() ==> $xx<=$fx);Cx(() ==> $xx>$fx);Cx(() ==> $xx>=$fx);Cx(() ==> $xx==$fx);Cx(() ==> $xx===$fx);
    Ix(() ==> $xx<=>$fx);
  begin_row('ca', 'WRAPA');
    Cx(() ==> $xx<$cx);Cx(() ==> $xx<=$cx);Cx(() ==> $xx>$cx);Cx(() ==> $xx>=$cx);Cx(() ==> $xx==$cx);Cx(() ==> $xx===$cx);
    Ix(() ==> $xx<=>$cx);
  begin_row('pa', 'WRAPA');
    Cx(() ==> $xx<$px);Cx(() ==> $xx<=$px);Cx(() ==> $xx>$px);Cx(() ==> $xx>=$px);Cx(() ==> $xx==$px);Cx(() ==> $xx===$px);
    Ix(() ==> $xx<=>$px);

  begin_row('va', 'WRAPO');
    Cx(() ==> $xy<$vy);Cx(() ==> $xy<=$vy);Cx(() ==> $xy>$vy);Cx(() ==> $xy>=$vy);Cx(() ==> $xy==$vy);Cx(() ==> $xy===$vy);
    Ix(() ==> $xy<=>$vy);
  begin_row('oa', 'WRAPO');
    Cx(() ==> $xy<$oy);Cx(() ==> $xy<=$oy);Cx(() ==> $xy>$oy);Cx(() ==> $xy>=$oy);Cx(() ==> $xy==$oy);Cx(() ==> $xy===$oy);
    Ix(() ==> $xy<=>$oy);
  begin_row('fa', 'WRAPO');
    Cx(() ==> $xy<$fy);Cx(() ==> $xy<=$fy);Cx(() ==> $xy>$fy);Cx(() ==> $xy>=$fy);Cx(() ==> $xy==$fy);Cx(() ==> $xy===$fy);
    Ix(() ==> $xy<=>$fy);
  begin_row('ca', 'WRAPO');
    Cx(() ==> $xy<$cy);Cx(() ==> $xy<=$cy);Cx(() ==> $xy>$cy);Cx(() ==> $xy>=$cy);Cx(() ==> $xy==$cy);Cx(() ==> $xy===$cy);
    Ix(() ==> $xy<=>$cy);
  begin_row('pa', 'WRAPO');
    Cx(() ==> $xy<$py);Cx(() ==> $xy<=$py);Cx(() ==> $xy>$py);Cx(() ==> $xy>=$py);Cx(() ==> $xy==$py);Cx(() ==> $xy===$py);
    Ix(() ==> $xy<=>$py);

  begin_row('va', 'WRAPD');
    Cx(() ==> $xz<$vz);Cx(() ==> $xz<=$vz);Cx(() ==> $xz>$vz);Cx(() ==> $xz>=$vz);Cx(() ==> $xz==$vz);Cx(() ==> $xz===$vz);
    Ix(() ==> $xz<=>$vz);
  begin_row('oa', 'WRAPD');
    Cx(() ==> $xz<$oz);Cx(() ==> $xz<=$oz);Cx(() ==> $xz>$oz);Cx(() ==> $xz>=$oz);Cx(() ==> $xz==$oz);Cx(() ==> $xz===$oz);
    Ix(() ==> $xz<=>$oz);
  begin_row('fa', 'WRAPD');
    Cx(() ==> $xz<$fz);Cx(() ==> $xz<=$fz);Cx(() ==> $xz>$fz);Cx(() ==> $xz>=$fz);Cx(() ==> $xz==$fz);Cx(() ==> $xz===$fz);
    Ix(() ==> $xz<=>$fz);
  begin_row('ca', 'WRAPD');
    Cx(() ==> $xz<$cz);Cx(() ==> $xz<=$cz);Cx(() ==> $xz>$cz);Cx(() ==> $xz>=$cz);Cx(() ==> $xz==$cz);Cx(() ==> $xz===$cz);
    Ix(() ==> $xz<=>$cz);
  begin_row('pa', 'WRAPD');
    Cx(() ==> $xz<$pz);Cx(() ==> $xz<=$pz);Cx(() ==> $xz>$pz);Cx(() ==> $xz>=$pz);Cx(() ==> $xz==$pz);Cx(() ==> $xz===$pz);
    Ix(() ==> $xz<=>$pz);
  print_footer();

  print_header('[dynamic] VAR ? $cm');
  begin_row('va');
    Cx(() ==> $va<$cm);Cx(() ==> $va<=$cm);Cx(() ==> $va>$cm);Cx(() ==> $va>=$cm);Cx(() ==> $va==$cm);Cx(() ==> $va===$cm);
    Ix(() ==> $va<=>$cm);
  begin_row('oa');
    Cx(() ==> $oa<$cm);Cx(() ==> $oa<=$cm);Cx(() ==> $oa>$cm);Cx(() ==> $oa>=$cm);Cx(() ==> $oa==$cm);Cx(() ==> $oa===$cm);
    Ix(() ==> $oa<=>$cm);
  begin_row('fa');
    Cx(() ==> $fa<$cm);Cx(() ==> $fa<=$cm);Cx(() ==> $fa>$cm);Cx(() ==> $fa>=$cm);Cx(() ==> $fa==$cm);Cx(() ==> $fa===$cm);
    Ix(() ==> $fa<=>$cm);
  begin_row('ca');
    Cx(() ==> $ca<$cm);Cx(() ==> $ca<=$cm);Cx(() ==> $ca>$cm);Cx(() ==> $ca>=$cm);Cx(() ==> $ca==$cm);Cx(() ==> $ca===$cm);
    Ix(() ==> $ca<=>$cm);
  begin_row('pa');
    Cx(() ==> $pa<$cm);Cx(() ==> $pa<=$cm);Cx(() ==> $pa>$cm);Cx(() ==> $pa>=$cm);Cx(() ==> $pa==$cm);Cx(() ==> $pa===$cm);
    Ix(() ==> $pa<=>$cm);

  begin_row('va', 'WRAPA');
    Cx(() ==> $vx<$xx);Cx(() ==> $vx<=$xx);Cx(() ==> $vx>$xx);Cx(() ==> $vx>=$xx);Cx(() ==> $vx==$xx);Cx(() ==> $vx===$xx);
    Ix(() ==> $vx<=>$xx);
  begin_row('oa', 'WRAPA');
    Cx(() ==> $ox<$xx);Cx(() ==> $ox<=$xx);Cx(() ==> $ox>$xx);Cx(() ==> $ox>=$xx);Cx(() ==> $ox==$xx);Cx(() ==> $ox===$xx);
    Ix(() ==> $ox<=>$xx);
  begin_row('fa', 'WRAPA');
    Cx(() ==> $fx<$xx);Cx(() ==> $fx<=$xx);Cx(() ==> $fx>$xx);Cx(() ==> $fx>=$xx);Cx(() ==> $fx==$xx);Cx(() ==> $fx===$xx);
    Ix(() ==> $fx<=>$xx);
  begin_row('ca', 'WRAPA');
    Cx(() ==> $cx<$xx);Cx(() ==> $cx<=$xx);Cx(() ==> $cx>$xx);Cx(() ==> $cx>=$xx);Cx(() ==> $cx==$xx);Cx(() ==> $cx===$xx);
    Ix(() ==> $cx<=>$xx);
  begin_row('pa', 'WRAPA');
    Cx(() ==> $px<$xx);Cx(() ==> $px<=$xx);Cx(() ==> $px>$xx);Cx(() ==> $px>=$xx);Cx(() ==> $px==$xx);Cx(() ==> $px===$xx);
    Ix(() ==> $px<=>$xx);

  begin_row('va', 'WRAPO');
    Cx(() ==> $vy<$xy);Cx(() ==> $vy<=$xy);Cx(() ==> $vy>$xy);Cx(() ==> $vy>=$xy);Cx(() ==> $vy==$xy);Cx(() ==> $vy===$xy);
    Ix(() ==> $vy<=>$xy);
  begin_row('oa', 'WRAPO');
    Cx(() ==> $oy<$xy);Cx(() ==> $oy<=$xy);Cx(() ==> $oy>$xy);Cx(() ==> $oy>=$xy);Cx(() ==> $oy==$xy);Cx(() ==> $oy===$xy);
    Ix(() ==> $oy<=>$xy);
  begin_row('fa', 'WRAPO');
    Cx(() ==> $fy<$xy);Cx(() ==> $fy<=$xy);Cx(() ==> $fy>$xy);Cx(() ==> $fy>=$xy);Cx(() ==> $fy==$xy);Cx(() ==> $fy===$xy);
    Ix(() ==> $fy<=>$xy);
  begin_row('ca', 'WRAPO');
    Cx(() ==> $cy<$xy);Cx(() ==> $cy<=$xy);Cx(() ==> $cy>$xy);Cx(() ==> $cy>=$xy);Cx(() ==> $cy==$xy);Cx(() ==> $cy===$xy);
    Ix(() ==> $cy<=>$xy);
  begin_row('pa', 'WRAPO');
    Cx(() ==> $py<$xy);Cx(() ==> $py<=$xy);Cx(() ==> $py>$xy);Cx(() ==> $py>=$xy);Cx(() ==> $py==$xy);Cx(() ==> $py===$xy);
    Ix(() ==> $py<=>$xy);

  begin_row('va', 'WRAPD');
    Cx(() ==> $vz<$xz);Cx(() ==> $vz<=$xz);Cx(() ==> $vz>$xz);Cx(() ==> $vz>=$xz);Cx(() ==> $vz==$xz);Cx(() ==> $vz===$xz);
    Ix(() ==> $vz<=>$xz);
  begin_row('oa', 'WRAPD');
    Cx(() ==> $oz<$xz);Cx(() ==> $oz<=$xz);Cx(() ==> $oz>$xz);Cx(() ==> $oz>=$xz);Cx(() ==> $oz==$xz);Cx(() ==> $oz===$xz);
    Ix(() ==> $oz<=>$xz);
  begin_row('fa', 'WRAPD');
    Cx(() ==> $fz<$xz);Cx(() ==> $fz<=$xz);Cx(() ==> $fz>$xz);Cx(() ==> $fz>=$xz);Cx(() ==> $fz==$xz);Cx(() ==> $fz===$xz);
    Ix(() ==> $fz<=>$xz);
  begin_row('ca', 'WRAPD');
    Cx(() ==> $cz<$xz);Cx(() ==> $cz<=$xz);Cx(() ==> $cz>$xz);Cx(() ==> $cz>=$xz);Cx(() ==> $cz==$xz);Cx(() ==> $cz===$xz);
    Ix(() ==> $cz<=>$xz);
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
