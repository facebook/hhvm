<?hh

class Info { public static bool $SawError = false; }
function handle_error($_errno, $msg, ...) :mixed{
  if (Info::$SawError) return false;
  if (
    !preg_match('/Implicit clsmeth to [^ ]+ conversion/', $msg) &&
    strpos($msg, 'Comparing clsmeth with vec') === false &&
    strpos($msg, 'Comparing clsmeth with non-clsmeth') === false &&
    strpos($msg, 'Comparing clsmeth with clsmeth relationally') === false
  ) {
    return false;
  }
  Info::$SawError = true;
  return true;
}

class Foo { static function bar() :mixed{} static function baz() :mixed{} }
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
  echo "+------------+------+------+------+------+------+------+------+\n";
  echo "| VAR        | <    | <=   | >    | >=   | ==   | ===  | <=>  |\n";
  echo "+============+======+======+======+======+======+======+======+";
}
<<__NEVER_INLINE>> function begin_row($var, $wrap = null) :mixed{
  printf("\n| %-10s |", $wrap !== null ? $wrap."(\$$var)" : "\$$var");
}
<<__NEVER_INLINE>> function C(bool $v) :mixed{
  printf(" %-4s |", ($v ? 'T' : 'F').(Info::$SawError ? '*' : ''));
  Info::$SawError = false;
}
<<__NEVER_INLINE>> function Cx($f) :mixed{
  try {
    C($f());
  } catch (InvalidOperationException $e) {
    print(" EXN  |");
    Info::$SawError = false;
  }
}
<<__NEVER_INLINE>> function I(int $v) :mixed{
  printf(" %-2d%s  |", $v, Info::$SawError ? '*' : ' ');
  Info::$SawError = false;
}
<<__NEVER_INLINE>> function Ix($f) :mixed{
  try {
    I($f());
  } catch (InvalidOperationException $e) {
    print(" EXN  |");
    Info::$SawError = false;
  }
}
<<__NEVER_INLINE>> function print_footer() :mixed{
  echo "\n+------------+------+------+------+------+------+------+------+\n\n";
}

<<__NEVER_INLINE>> function static_compare() :mixed{
  $cm = Foo::bar<>;
  $nv = null;
  $tv = true;
  $bv = false;
  $iv = 42;
  $fv = 3.14159;
  $sv = 'Foo::bar';
  $rv = opendir(getcwd());
  $ov = new StrObj('Foo::bar');
  $va = vec[Foo::class, 'bar'];
  $da = dict[0 => Foo::class, 1 => 'bar'];
  $cp = Foo::bar<>;
  $ep = bar<>;
  $lp = Foo::baz<>;
  $qp = CLS('Foo');

  $xx = vec[$cm]; $nx = vec[$nv]; $tx = vec[$tv]; $bx = vec[$bv];
  $ix = vec[$iv]; $fx = vec[$fv]; $sx = vec[$sv]; $rx = vec[$rv];
  $ox = vec[$ov]; $vx = vec[$va]; $dx = vec[$da]; $cx = vec[$cp];
  $ex = vec[$ep]; $lx = vec[$lp]; $qx = vec[$qp];

  $xy = new Wrapper($cm); $ny = new Wrapper($nv); $ty = new Wrapper($tv);
  $by = new Wrapper($bv); $iy = new Wrapper($iv); $fy = new Wrapper($fv);
  $sy = new Wrapper($sv); $ry = new Wrapper($rv); $oy = new Wrapper($ov);
  $vy = new Wrapper($va); $dy = new Wrapper($da); $cy = new Wrapper($cp);
  $ey = new Wrapper($ep); $ly = new Wrapper($lp); $qy = new Wrapper($qp);

  $xz = new stdClass; $xz->v = $cm; $nz = new stdClass; $nz->v = $nv;
  $tz = new stdClass; $tz->v = $tv; $bz = new stdClass; $bz->v = $bv;
  $iz = new stdClass; $iz->v = $iv; $fz = new stdClass; $fz->v = $fv;
  $sz = new stdClass; $sz->v = $sv; $rz = new stdClass; $rz->v = $rv;
  $oz = new stdClass; $oz->v = $ov; $vz = new stdClass; $vz->v = $va;
  $dz = new stdClass; $dz->v = $da; $cz = new stdClass; $cz->v = $cp;
  $ez = new stdClass; $ez->v = $ep; $lz = new stdClass; $lz->v = $lp;
  $qz = new stdClass; $qz->v = $qp;

  print_header('[static] $cm ? VAR');
  begin_row('cm');
    Cx(() ==> $cm<$cm);Cx(() ==> $cm<=$cm);Cx(() ==> $cm>$cm);Cx(() ==> $cm>=$cm);Cx(() ==> $cm==$cm);Cx(() ==> $cm===$cm);
    Ix(() ==> $cm<=>$cm);
  begin_row('nv');
    Cx(() ==> $cm<$nv);Cx(() ==> $cm<=$nv);Cx(() ==> $cm>$nv);Cx(() ==> $cm>=$nv);Cx(() ==> $cm==$nv);Cx(() ==> $cm===$nv);
    Ix(() ==> $cm<=>$nv);
  begin_row('tv');
    Cx(() ==> $cm<$tv);Cx(() ==> $cm<=$tv);Cx(() ==> $cm>$tv);Cx(() ==> $cm>=$tv);Cx(() ==> $cm==$tv);Cx(() ==> $cm===$tv);
    Ix(() ==> $cm<=>$tv);
  begin_row('bv');
    Cx(() ==> $cm<$bv);Cx(() ==> $cm<=$bv);Cx(() ==> $cm>$bv);Cx(() ==> $cm>=$bv);Cx(() ==> $cm==$bv);Cx(() ==> $cm===$bv);
    Ix(() ==> $cm<=>$bv);
  begin_row('iv');
    Cx(() ==> $cm<$iv);Cx(() ==> $cm<=$iv);Cx(() ==> $cm>$iv);Cx(() ==> $cm>=$iv);Cx(() ==> $cm==$iv);Cx(() ==> $cm===$iv);
    Ix(() ==> $cm<=>$iv);
  begin_row('fv');
    Cx(() ==> $cm<$fv);Cx(() ==> $cm<=$fv);Cx(() ==> $cm>$fv);Cx(() ==> $cm>=$fv);Cx(() ==> $cm==$fv);Cx(() ==> $cm===$fv);
    Ix(() ==> $cm<=>$fv);
  begin_row('sv');
    Cx(() ==> $cm<$sv);Cx(() ==> $cm<=$sv);Cx(() ==> $cm>$sv);Cx(() ==> $cm>=$sv);Cx(() ==> $cm==$sv);Cx(() ==> $cm===$sv);
    Ix(() ==> $cm<=>$sv);
  begin_row('rv');
    Cx(() ==> $cm<$rv);Cx(() ==> $cm<=$rv);Cx(() ==> $cm>$rv);Cx(() ==> $cm>=$rv);Cx(() ==> $cm==$rv);Cx(() ==> $cm===$rv);
    Ix(() ==> $cm<=>$rv);
  begin_row('ov');
    Cx(() ==> $cm<$ov);Cx(() ==> $cm<=$ov);Cx(() ==> $cm>$ov);Cx(() ==> $cm>=$ov);Cx(() ==> $cm==$ov);Cx(() ==> $cm===$ov);
    Ix(() ==> $cm<=>$ov);
  begin_row('va');
    Cx(() ==> $cm<$va);Cx(() ==> $cm<=$va);Cx(() ==> $cm>$va);Cx(() ==> $cm>=$va);Cx(() ==> $cm==$va);Cx(() ==> $cm===$va);
    Ix(() ==> $cm<=>$va);
  begin_row('cp');
    Cx(() ==> $cm<$cp);Cx(() ==> $cm<=$cp);Cx(() ==> $cm>$cp);Cx(() ==> $cm>=$cp);Cx(() ==> $cm==$cp);Cx(() ==> $cm===$cp);
    Ix(() ==> $cm<=>$cp);
  begin_row('ep');
    Cx(() ==> $cm<$ep);Cx(() ==> $cm<=$ep);Cx(() ==> $cm>$ep);Cx(() ==> $cm>=$ep);Cx(() ==> $cm==$ep);Cx(() ==> $cm===$ep);
    Ix(() ==> $cm<=>$ep);
  begin_row('lp');
    Cx(() ==> $cm<$lp);Cx(() ==> $cm<=$lp);Cx(() ==> $cm>$lp);Cx(() ==> $cm>=$lp);Cx(() ==> $cm==$lp);Cx(() ==> $cm===$lp);
    Ix(() ==> $cm<=>$lp);
  begin_row('qp');
    Cx(() ==> $cm<$qp);Cx(() ==> $cm<=$qp);Cx(() ==> $cm>$qp);Cx(() ==> $cm>=$qp);Cx(() ==> $cm==$qp);Cx(() ==> $cm===$qp);
    Ix(() ==> $cm<=>$qp);

  begin_row('nv', 'WRAPA');
    Cx(() ==> $xx<$nx);Cx(() ==> $xx<=$nx);Cx(() ==> $xx>$nx);Cx(() ==> $xx>=$nx);Cx(() ==> $xx==$nx);Cx(() ==> $xx===$nx);
    Ix(() ==> $xx<=>$nx);
  begin_row('tv', 'WRAPA');
    Cx(() ==> $xx<$tx);Cx(() ==> $xx<=$tx);Cx(() ==> $xx>$tx);Cx(() ==> $xx>=$tx);Cx(() ==> $xx==$tx);Cx(() ==> $xx===$tx);
    Ix(() ==> $xx<=>$tx);
  begin_row('bv', 'WRAPA');
    Cx(() ==> $xx<$bx);Cx(() ==> $xx<=$bx);Cx(() ==> $xx>$bx);Cx(() ==> $xx>=$bx);Cx(() ==> $xx==$bx);Cx(() ==> $xx===$bx);
    Ix(() ==> $xx<=>$bx);
  begin_row('iv', 'WRAPA');
    Cx(() ==> $xx<$ix);Cx(() ==> $xx<=$ix);Cx(() ==> $xx>$ix);Cx(() ==> $xx>=$ix);Cx(() ==> $xx==$ix);Cx(() ==> $xx===$ix);
    Ix(() ==> $xx<=>$ix);
  begin_row('fv', 'WRAPA');
    Cx(() ==> $xx<$fx);Cx(() ==> $xx<=$fx);Cx(() ==> $xx>$fx);Cx(() ==> $xx>=$fx);Cx(() ==> $xx==$fx);Cx(() ==> $xx===$fx);
    Ix(() ==> $xx<=>$fx);
  begin_row('sv', 'WRAPA');
    Cx(() ==> $xx<$sx);Cx(() ==> $xx<=$sx);Cx(() ==> $xx>$sx);Cx(() ==> $xx>=$sx);Cx(() ==> $xx==$sx);Cx(() ==> $xx===$sx);
    Ix(() ==> $xx<=>$sx);
  begin_row('rv', 'WRAPA');
    Cx(() ==> $xx<$rx);Cx(() ==> $xx<=$rx);Cx(() ==> $xx>$rx);Cx(() ==> $xx>=$rx);Cx(() ==> $xx==$rx);Cx(() ==> $xx===$rx);
    Ix(() ==> $xx<=>$rx);
  begin_row('ov', 'WRAPA');
    Cx(() ==> $xx<$ox);Cx(() ==> $xx<=$ox);Cx(() ==> $xx>$ox);Cx(() ==> $xx>=$ox);Cx(() ==> $xx==$ox);Cx(() ==> $xx===$ox);
    Ix(() ==> $xx<=>$ox);
  begin_row('va', 'WRAPA');
    Cx(() ==> $xx<$vx);Cx(() ==> $xx<=$vx);Cx(() ==> $xx>$vx);Cx(() ==> $xx>=$vx);Cx(() ==> $xx==$vx);Cx(() ==> $xx===$vx);
    Ix(() ==> $xx<=>$vx);
  begin_row('cp', 'WRAPA');
    Cx(() ==> $xx<$cx);Cx(() ==> $xx<=$cx);Cx(() ==> $xx>$cx);Cx(() ==> $xx>=$cx);Cx(() ==> $xx==$cx);Cx(() ==> $xx===$cx);
    Ix(() ==> $xx<=>$cx);
  begin_row('ep', 'WRAPA');
    Cx(() ==> $xx<$ex);Cx(() ==> $xx<=$ex);Cx(() ==> $xx>$ex);Cx(() ==> $xx>=$ex);Cx(() ==> $xx==$ex);Cx(() ==> $xx===$ex);
    Ix(() ==> $xx<=>$ex);
  begin_row('lp', 'WRAPA');
    Cx(() ==> $xx<$lx);Cx(() ==> $xx<=$lx);Cx(() ==> $xx>$lx);Cx(() ==> $xx>=$lx);Cx(() ==> $xx==$lx);Cx(() ==> $xx===$lx);
    Ix(() ==> $xx<=>$lx);
  begin_row('qp', 'WRAPA');
    Cx(() ==> $xx<$qx);Cx(() ==> $xx<=$qx);Cx(() ==> $xx>$qx);Cx(() ==> $xx>=$qx);Cx(() ==> $xx==$qx);Cx(() ==> $xx===$qx);
    Ix(() ==> $xx<=>$qx);

  begin_row('nv', 'WRAPO');
    Cx(() ==> $xy<$ny);Cx(() ==> $xy<=$ny);Cx(() ==> $xy>$ny);Cx(() ==> $xy>=$ny);Cx(() ==> $xy==$ny);Cx(() ==> $xy===$ny);
    Ix(() ==> $xy<=>$ny);
  begin_row('tv', 'WRAPO');
    Cx(() ==> $xy<$ty);Cx(() ==> $xy<=$ty);Cx(() ==> $xy>$ty);Cx(() ==> $xy>=$ty);Cx(() ==> $xy==$ty);Cx(() ==> $xy===$ty);
    Ix(() ==> $xy<=>$ty);
  begin_row('bv', 'WRAPO');
    Cx(() ==> $xy<$by);Cx(() ==> $xy<=$by);Cx(() ==> $xy>$by);Cx(() ==> $xy>=$by);Cx(() ==> $xy==$by);Cx(() ==> $xy===$by);
    Ix(() ==> $xy<=>$by);
  begin_row('iv', 'WRAPO');
    Cx(() ==> $xy<$iy);Cx(() ==> $xy<=$iy);Cx(() ==> $xy>$iy);Cx(() ==> $xy>=$iy);Cx(() ==> $xy==$iy);Cx(() ==> $xy===$iy);
    Ix(() ==> $xy<=>$iy);
  begin_row('fv', 'WRAPO');
    Cx(() ==> $xy<$fy);Cx(() ==> $xy<=$fy);Cx(() ==> $xy>$fy);Cx(() ==> $xy>=$fy);Cx(() ==> $xy==$fy);Cx(() ==> $xy===$fy);
    Ix(() ==> $xy<=>$fy);
  begin_row('sv', 'WRAPO');
    Cx(() ==> $xy<$sy);Cx(() ==> $xy<=$sy);Cx(() ==> $xy>$sy);Cx(() ==> $xy>=$sy);Cx(() ==> $xy==$sy);Cx(() ==> $xy===$sy);
    Ix(() ==> $xy<=>$sy);
  begin_row('rv', 'WRAPO');
    Cx(() ==> $xy<$ry);Cx(() ==> $xy<=$ry);Cx(() ==> $xy>$ry);Cx(() ==> $xy>=$ry);Cx(() ==> $xy==$ry);Cx(() ==> $xy===$ry);
    Ix(() ==> $xy<=>$ry);
  begin_row('ov', 'WRAPO');
    Cx(() ==> $xy<$oy);Cx(() ==> $xy<=$oy);Cx(() ==> $xy>$oy);Cx(() ==> $xy>=$oy);Cx(() ==> $xy==$oy);Cx(() ==> $xy===$oy);
    Ix(() ==> $xy<=>$oy);
  begin_row('va', 'WRAPO');
    Cx(() ==> $xy<$vy);Cx(() ==> $xy<=$vy);Cx(() ==> $xy>$vy);Cx(() ==> $xy>=$vy);Cx(() ==> $xy==$vy);Cx(() ==> $xy===$vy);
    Ix(() ==> $xy<=>$vy);
  begin_row('cp', 'WRAPO');
    Cx(() ==> $xy<$cy);Cx(() ==> $xy<=$cy);Cx(() ==> $xy>$cy);Cx(() ==> $xy>=$cy);Cx(() ==> $xy==$cy);Cx(() ==> $xy===$cy);
    Ix(() ==> $xy<=>$cy);
  begin_row('ep', 'WRAPO');
    Cx(() ==> $xy<$ey);Cx(() ==> $xy<=$ey);Cx(() ==> $xy>$ey);Cx(() ==> $xy>=$ey);Cx(() ==> $xy==$ey);Cx(() ==> $xy===$ey);
    Ix(() ==> $xy<=>$ey);
  begin_row('lp', 'WRAPO');
    Cx(() ==> $xy<$ly);Cx(() ==> $xy<=$ly);Cx(() ==> $xy>$ly);Cx(() ==> $xy>=$ly);Cx(() ==> $xy==$ly);Cx(() ==> $xy===$ly);
    Ix(() ==> $xy<=>$ly);
  begin_row('qp', 'WRAPO');
    Cx(() ==> $xy<$qy);Cx(() ==> $xy<=$qy);Cx(() ==> $xy>$qy);Cx(() ==> $xy>=$qy);Cx(() ==> $xy==$qy);Cx(() ==> $xy===$qy);
    Ix(() ==> $xy<=>$qy);

  begin_row('nv', 'WRAPD');
    Cx(() ==> $xz<$nz);Cx(() ==> $xz<=$nz);Cx(() ==> $xz>$nz);Cx(() ==> $xz>=$nz);Cx(() ==> $xz==$nz);Cx(() ==> $xz===$nz);
    Ix(() ==> $xz<=>$nz);
  begin_row('tv', 'WRAPD');
    Cx(() ==> $xz<$tz);Cx(() ==> $xz<=$tz);Cx(() ==> $xz>$tz);Cx(() ==> $xz>=$tz);Cx(() ==> $xz==$tz);Cx(() ==> $xz===$tz);
    Ix(() ==> $xz<=>$tz);
  begin_row('bv', 'WRAPD');
    Cx(() ==> $xz<$bz);Cx(() ==> $xz<=$bz);Cx(() ==> $xz>$bz);Cx(() ==> $xz>=$bz);Cx(() ==> $xz==$bz);Cx(() ==> $xz===$bz);
    Ix(() ==> $xz<=>$bz);
  begin_row('iv', 'WRAPD');
    Cx(() ==> $xz<$iz);Cx(() ==> $xz<=$iz);Cx(() ==> $xz>$iz);Cx(() ==> $xz>=$iz);Cx(() ==> $xz==$iz);Cx(() ==> $xz===$iz);
    Ix(() ==> $xz<=>$iz);
  begin_row('fv', 'WRAPD');
    Cx(() ==> $xz<$fz);Cx(() ==> $xz<=$fz);Cx(() ==> $xz>$fz);Cx(() ==> $xz>=$fz);Cx(() ==> $xz==$fz);Cx(() ==> $xz===$fz);
    Ix(() ==> $xz<=>$fz);
  begin_row('sv', 'WRAPD');
    Cx(() ==> $xz<$sz);Cx(() ==> $xz<=$sz);Cx(() ==> $xz>$sz);Cx(() ==> $xz>=$sz);Cx(() ==> $xz==$sz);Cx(() ==> $xz===$sz);
    Ix(() ==> $xz<=>$sz);
  begin_row('rv', 'WRAPD');
    Cx(() ==> $xz<$rz);Cx(() ==> $xz<=$rz);Cx(() ==> $xz>$rz);Cx(() ==> $xz>=$rz);Cx(() ==> $xz==$rz);Cx(() ==> $xz===$rz);
    Ix(() ==> $xz<=>$rz);
  begin_row('ov', 'WRAPD');
    Cx(() ==> $xz<$oz);Cx(() ==> $xz<=$oz);Cx(() ==> $xz>$oz);Cx(() ==> $xz>=$oz);Cx(() ==> $xz==$oz);Cx(() ==> $xz===$oz);
    Ix(() ==> $xz<=>$oz);
  begin_row('va', 'WRAPD');
    Cx(() ==> $xz<$vz);Cx(() ==> $xz<=$vz);Cx(() ==> $xz>$vz);Cx(() ==> $xz>=$vz);Cx(() ==> $xz==$vz);Cx(() ==> $xz===$vz);
    Ix(() ==> $xz<=>$vz);
  begin_row('cp', 'WRAPD');
    Cx(() ==> $xz<$cz);Cx(() ==> $xz<=$cz);Cx(() ==> $xz>$cz);Cx(() ==> $xz>=$cz);Cx(() ==> $xz==$cz);Cx(() ==> $xz===$cz);
    Ix(() ==> $xz<=>$cz);
  begin_row('ep', 'WRAPD');
    Cx(() ==> $xz<$ez);Cx(() ==> $xz<=$ez);Cx(() ==> $xz>$ez);Cx(() ==> $xz>=$ez);Cx(() ==> $xz==$ez);Cx(() ==> $xz===$ez);
    Ix(() ==> $xz<=>$ez);
  begin_row('lp', 'WRAPD');
    Cx(() ==> $xz<$lz);Cx(() ==> $xz<=$lz);Cx(() ==> $xz>$lz);Cx(() ==> $xz>=$lz);Cx(() ==> $xz==$lz);Cx(() ==> $xz===$lz);
    Ix(() ==> $xz<=>$lz);
  begin_row('qp', 'WRAPD');
    Cx(() ==> $xz<$qz);Cx(() ==> $xz<=$qz);Cx(() ==> $xz>$qz);Cx(() ==> $xz>=$qz);Cx(() ==> $xz==$qz);Cx(() ==> $xz===$qz);
    Ix(() ==> $xz<=>$qz);
  print_footer();

  print_header('[static] VAR ? $cm');
  begin_row('cm');
    Cx(() ==> $cm<$cm);Cx(() ==> $cm<=$cm);Cx(() ==> $cm>$cm);Cx(() ==> $cm>=$cm);Cx(() ==> $cm==$cm);Cx(() ==> $cm===$cm);
    Ix(() ==> $cm<=>$cm);
  begin_row('nv');
    Cx(() ==> $nv<$cm);Cx(() ==> $nv<=$cm);Cx(() ==> $nv>$cm);Cx(() ==> $nv>=$cm);Cx(() ==> $nv==$cm);Cx(() ==> $nv===$cm);
    Ix(() ==> $nv<=>$cm);
  begin_row('tv');
    Cx(() ==> $tv<$cm);Cx(() ==> $tv<=$cm);Cx(() ==> $tv>$cm);Cx(() ==> $tv>=$cm);Cx(() ==> $tv==$cm);Cx(() ==> $tv===$cm);
    Ix(() ==> $tv<=>$cm);
  begin_row('bv');
    Cx(() ==> $bv<$cm);Cx(() ==> $bv<=$cm);Cx(() ==> $bv>$cm);Cx(() ==> $bv>=$cm);Cx(() ==> $bv==$cm);Cx(() ==> $bv===$cm);
    Ix(() ==> $bv<=>$cm);
  begin_row('iv');
    Cx(() ==> $iv<$cm);Cx(() ==> $iv<=$cm);Cx(() ==> $iv>$cm);Cx(() ==> $iv>=$cm);Cx(() ==> $iv==$cm);Cx(() ==> $iv===$cm);
    Ix(() ==> $iv<=>$cm);
  begin_row('fv');
    Cx(() ==> $fv<$cm);Cx(() ==> $fv<=$cm);Cx(() ==> $fv>$cm);Cx(() ==> $fv>=$cm);Cx(() ==> $fv==$cm);Cx(() ==> $fv===$cm);
    Ix(() ==> $fv<=>$cm);
  begin_row('sv');
    Cx(() ==> $sv<$cm);Cx(() ==> $sv<=$cm);Cx(() ==> $sv>$cm);Cx(() ==> $sv>=$cm);Cx(() ==> $sv==$cm);Cx(() ==> $sv===$cm);
    Ix(() ==> $sv<=>$cm);
  begin_row('rv');
    Cx(() ==> $rv<$cm);Cx(() ==> $rv<=$cm);Cx(() ==> $rv>$cm);Cx(() ==> $rv>=$cm);Cx(() ==> $rv==$cm);Cx(() ==> $rv===$cm);
    Ix(() ==> $rv<=>$cm);
  begin_row('ov');
    Cx(() ==> $ov<$cm);Cx(() ==> $ov<=$cm);Cx(() ==> $ov>$cm);Cx(() ==> $ov>=$cm);Cx(() ==> $ov==$cm);Cx(() ==> $ov===$cm);
    Ix(() ==> $ov<=>$cm);
  begin_row('va');
    Cx(() ==> $va<$cm);Cx(() ==> $va<=$cm);Cx(() ==> $va>$cm);Cx(() ==> $va>=$cm);Cx(() ==> $va==$cm);Cx(() ==> $va===$cm);
    Ix(() ==> $va<=>$cm);
  begin_row('cp');
    Cx(() ==> $cp<$cm);Cx(() ==> $cp<=$cm);Cx(() ==> $cp>$cm);Cx(() ==> $cp>=$cm);Cx(() ==> $cp==$cm);Cx(() ==> $cp===$cm);
    Ix(() ==> $cp<=>$cm);
  begin_row('ep');
    Cx(() ==> $ep<$cm);Cx(() ==> $ep<=$cm);Cx(() ==> $ep>$cm);Cx(() ==> $ep>=$cm);Cx(() ==> $ep==$cm);Cx(() ==> $ep===$cm);
    Ix(() ==> $ep<=>$cm);
  begin_row('lp');
    Cx(() ==> $lp<$cm);Cx(() ==> $lp<=$cm);Cx(() ==> $lp>$cm);Cx(() ==> $lp>=$cm);Cx(() ==> $lp==$cm);Cx(() ==> $lp===$cm);
    Ix(() ==> $lp<=>$cm);
  begin_row('qp');
    Cx(() ==> $qp<$cm);Cx(() ==> $qp<=$cm);Cx(() ==> $qp>$cm);Cx(() ==> $qp>=$cm);Cx(() ==> $qp==$cm);Cx(() ==> $qp===$cm);
    Ix(() ==> $qp<=>$cm);

  begin_row('nv', 'WRAPA');
    Cx(() ==> $nx<$xx);Cx(() ==> $nx<=$xx);Cx(() ==> $nx>$xx);Cx(() ==> $nx>=$xx);Cx(() ==> $nx==$xx);Cx(() ==> $nx===$xx);
    Ix(() ==> $nx<=>$xx);
  begin_row('tv', 'WRAPA');
    Cx(() ==> $tx<$xx);Cx(() ==> $tx<=$xx);Cx(() ==> $tx>$xx);Cx(() ==> $tx>=$xx);Cx(() ==> $tx==$xx);Cx(() ==> $tx===$xx);
    Ix(() ==> $tx<=>$xx);
  begin_row('bv', 'WRAPA');
    Cx(() ==> $bx<$xx);Cx(() ==> $bx<=$xx);Cx(() ==> $bx>$xx);Cx(() ==> $bx>=$xx);Cx(() ==> $bx==$xx);Cx(() ==> $bx===$xx);
    Ix(() ==> $bx<=>$xx);
  begin_row('iv', 'WRAPA');
    Cx(() ==> $ix<$xx);Cx(() ==> $ix<=$xx);Cx(() ==> $ix>$xx);Cx(() ==> $ix>=$xx);Cx(() ==> $ix==$xx);Cx(() ==> $ix===$xx);
    Ix(() ==> $ix<=>$xx);
  begin_row('fv', 'WRAPA');
    Cx(() ==> $fx<$xx);Cx(() ==> $fx<=$xx);Cx(() ==> $fx>$xx);Cx(() ==> $fx>=$xx);Cx(() ==> $fx==$xx);Cx(() ==> $fx===$xx);
    Ix(() ==> $fx<=>$xx);
  begin_row('sv', 'WRAPA');
    Cx(() ==> $sx<$xx);Cx(() ==> $sx<=$xx);Cx(() ==> $sx>$xx);Cx(() ==> $sx>=$xx);Cx(() ==> $sx==$xx);Cx(() ==> $sx===$xx);
    Ix(() ==> $sx<=>$xx);
  begin_row('rv', 'WRAPA');
    Cx(() ==> $rx<$xx);Cx(() ==> $rx<=$xx);Cx(() ==> $rx>$xx);Cx(() ==> $rx>=$xx);Cx(() ==> $rx==$xx);Cx(() ==> $rx===$xx);
    Ix(() ==> $rx<=>$xx);
  begin_row('ov', 'WRAPA');
    Cx(() ==> $ox<$xx);Cx(() ==> $ox<=$xx);Cx(() ==> $ox>$xx);Cx(() ==> $ox>=$xx);Cx(() ==> $ox==$xx);Cx(() ==> $ox===$xx);
    Ix(() ==> $ox<=>$xx);
  begin_row('va', 'WRAPA');
    Cx(() ==> $vx<$xx);Cx(() ==> $vx<=$xx);Cx(() ==> $vx>$xx);Cx(() ==> $vx>=$xx);Cx(() ==> $vx==$xx);Cx(() ==> $vx===$xx);
    Ix(() ==> $vx<=>$xx);
  begin_row('cp', 'WRAPA');
    Cx(() ==> $cx<$xx);Cx(() ==> $cx<=$xx);Cx(() ==> $cx>$xx);Cx(() ==> $cx>=$xx);Cx(() ==> $cx==$xx);Cx(() ==> $cx===$xx);
    Ix(() ==> $cx<=>$xx);
  begin_row('ep', 'WRAPA');
    Cx(() ==> $ex<$xx);Cx(() ==> $ex<=$xx);Cx(() ==> $ex>$xx);Cx(() ==> $ex>=$xx);Cx(() ==> $ex==$xx);Cx(() ==> $ex===$xx);
    Ix(() ==> $ex<=>$xx);
  begin_row('lp', 'WRAPA');
    Cx(() ==> $lx<$xx);Cx(() ==> $lx<=$xx);Cx(() ==> $lx>$xx);Cx(() ==> $lx>=$xx);Cx(() ==> $lx==$xx);Cx(() ==> $lx===$xx);
    Ix(() ==> $lx<=>$xx);
  begin_row('qp', 'WRAPA');
    Cx(() ==> $qx<$xx);Cx(() ==> $qx<=$xx);Cx(() ==> $qx>$xx);Cx(() ==> $qx>=$xx);Cx(() ==> $qx==$xx);Cx(() ==> $qx===$xx);
    Ix(() ==> $qx<=>$xx);

  begin_row('nv', 'WRAPO');
    Cx(() ==> $ny<$xy);Cx(() ==> $ny<=$xy);Cx(() ==> $ny>$xy);Cx(() ==> $ny>=$xy);Cx(() ==> $ny==$xy);Cx(() ==> $ny===$xy);
    Ix(() ==> $ny<=>$xy);
  begin_row('tv', 'WRAPO');
    Cx(() ==> $ty<$xy);Cx(() ==> $ty<=$xy);Cx(() ==> $ty>$xy);Cx(() ==> $ty>=$xy);Cx(() ==> $ty==$xy);Cx(() ==> $ty===$xy);
    Ix(() ==> $ty<=>$xy);
  begin_row('bv', 'WRAPO');
    Cx(() ==> $by<$xy);Cx(() ==> $by<=$xy);Cx(() ==> $by>$xy);Cx(() ==> $by>=$xy);Cx(() ==> $by==$xy);Cx(() ==> $by===$xy);
    Ix(() ==> $by<=>$xy);
  begin_row('iv', 'WRAPO');
    Cx(() ==> $iy<$xy);Cx(() ==> $iy<=$xy);Cx(() ==> $iy>$xy);Cx(() ==> $iy>=$xy);Cx(() ==> $iy==$xy);Cx(() ==> $iy===$xy);
    Ix(() ==> $iy<=>$xy);
  begin_row('fv', 'WRAPO');
    Cx(() ==> $fy<$xy);Cx(() ==> $fy<=$xy);Cx(() ==> $fy>$xy);Cx(() ==> $fy>=$xy);Cx(() ==> $fy==$xy);Cx(() ==> $fy===$xy);
    Ix(() ==> $fy<=>$xy);
  begin_row('sv', 'WRAPO');
    Cx(() ==> $sy<$xy);Cx(() ==> $sy<=$xy);Cx(() ==> $sy>$xy);Cx(() ==> $sy>=$xy);Cx(() ==> $sy==$xy);Cx(() ==> $sy===$xy);
    Ix(() ==> $sy<=>$xy);
  begin_row('rv', 'WRAPO');
    Cx(() ==> $ry<$xy);Cx(() ==> $ry<=$xy);Cx(() ==> $ry>$xy);Cx(() ==> $ry>=$xy);Cx(() ==> $ry==$xy);Cx(() ==> $ry===$xy);
    Ix(() ==> $ry<=>$xy);
  begin_row('ov', 'WRAPO');
    Cx(() ==> $oy<$xy);Cx(() ==> $oy<=$xy);Cx(() ==> $oy>$xy);Cx(() ==> $oy>=$xy);Cx(() ==> $oy==$xy);Cx(() ==> $oy===$xy);
    Ix(() ==> $oy<=>$xy);
  begin_row('va', 'WRAPO');
    Cx(() ==> $vy<$xy);Cx(() ==> $vy<=$xy);Cx(() ==> $vy>$xy);Cx(() ==> $vy>=$xy);Cx(() ==> $vy==$xy);Cx(() ==> $vy===$xy);
    Ix(() ==> $vy<=>$xy);
  begin_row('cp', 'WRAPO');
    Cx(() ==> $cy<$xy);Cx(() ==> $cy<=$xy);Cx(() ==> $cy>$xy);Cx(() ==> $cy>=$xy);Cx(() ==> $cy==$xy);Cx(() ==> $cy===$xy);
    Ix(() ==> $cy<=>$xy);
  begin_row('ep', 'WRAPO');
    Cx(() ==> $ey<$xy);Cx(() ==> $ey<=$xy);Cx(() ==> $ey>$xy);Cx(() ==> $ey>=$xy);Cx(() ==> $ey==$xy);Cx(() ==> $ey===$xy);
    Ix(() ==> $ey<=>$xy);
  begin_row('lp', 'WRAPO');
    Cx(() ==> $ly<$xy);Cx(() ==> $ly<=$xy);Cx(() ==> $ly>$xy);Cx(() ==> $ly>=$xy);Cx(() ==> $ly==$xy);Cx(() ==> $ly===$xy);
    Ix(() ==> $ly<=>$xy);
  begin_row('qp', 'WRAPO');
    Cx(() ==> $qy<$xy);Cx(() ==> $qy<=$xy);Cx(() ==> $qy>$xy);Cx(() ==> $qy>=$xy);Cx(() ==> $qy==$xy);Cx(() ==> $qy===$xy);
    Ix(() ==> $qy<=>$xy);

  begin_row('nv', 'WRAPD');
    Cx(() ==> $nz<$xz);Cx(() ==> $nz<=$xz);Cx(() ==> $nz>$xz);Cx(() ==> $nz>=$xz);Cx(() ==> $nz==$xz);Cx(() ==> $nz===$xz);
    Ix(() ==> $nz<=>$xz);
  begin_row('tv', 'WRAPD');
    Cx(() ==> $tz<$xz);Cx(() ==> $tz<=$xz);Cx(() ==> $tz>$xz);Cx(() ==> $tz>=$xz);Cx(() ==> $tz==$xz);Cx(() ==> $tz===$xz);
    Ix(() ==> $tz<=>$xz);
  begin_row('bv', 'WRAPD');
    Cx(() ==> $bz<$xz);Cx(() ==> $bz<=$xz);Cx(() ==> $bz>$xz);Cx(() ==> $bz>=$xz);Cx(() ==> $bz==$xz);Cx(() ==> $bz===$xz);
    Ix(() ==> $bz<=>$xz);
  begin_row('iv', 'WRAPD');
    Cx(() ==> $iz<$xz);Cx(() ==> $iz<=$xz);Cx(() ==> $iz>$xz);Cx(() ==> $iz>=$xz);Cx(() ==> $iz==$xz);Cx(() ==> $iz===$xz);
    Ix(() ==> $iz<=>$xz);
  begin_row('fv', 'WRAPD');
    Cx(() ==> $fz<$xz);Cx(() ==> $fz<=$xz);Cx(() ==> $fz>$xz);Cx(() ==> $fz>=$xz);Cx(() ==> $fz==$xz);Cx(() ==> $fz===$xz);
    Ix(() ==> $fz<=>$xz);
  begin_row('sv', 'WRAPD');
    Cx(() ==> $sz<$xz);Cx(() ==> $sz<=$xz);Cx(() ==> $sz>$xz);Cx(() ==> $sz>=$xz);Cx(() ==> $sz==$xz);Cx(() ==> $sz===$xz);
    Ix(() ==> $sz<=>$xz);
  begin_row('rv', 'WRAPD');
    Cx(() ==> $rz<$xz);Cx(() ==> $rz<=$xz);Cx(() ==> $rz>$xz);Cx(() ==> $rz>=$xz);Cx(() ==> $rz==$xz);Cx(() ==> $rz===$xz);
    Ix(() ==> $rz<=>$xz);
  begin_row('ov', 'WRAPD');
    Cx(() ==> $oz<$xz);Cx(() ==> $oz<=$xz);Cx(() ==> $oz>$xz);Cx(() ==> $oz>=$xz);Cx(() ==> $oz==$xz);Cx(() ==> $oz===$xz);
    Ix(() ==> $oz<=>$xz);
  begin_row('va', 'WRAPD');
    Cx(() ==> $vz<$xz);Cx(() ==> $vz<=$xz);Cx(() ==> $vz>$xz);Cx(() ==> $vz>=$xz);Cx(() ==> $vz==$xz);Cx(() ==> $vz===$xz);
    Ix(() ==> $vz<=>$xz);
  begin_row('cp', 'WRAPD');
    Cx(() ==> $cz<$xz);Cx(() ==> $cz<=$xz);Cx(() ==> $cz>$xz);Cx(() ==> $cz>=$xz);Cx(() ==> $cz==$xz);Cx(() ==> $cz===$xz);
    Ix(() ==> $cz<=>$xz);
  begin_row('ep', 'WRAPD');
    Cx(() ==> $ez<$xz);Cx(() ==> $ez<=$xz);Cx(() ==> $ez>$xz);Cx(() ==> $ez>=$xz);Cx(() ==> $ez==$xz);Cx(() ==> $ez===$xz);
    Ix(() ==> $ez<=>$xz);
  begin_row('lp', 'WRAPD');
    Cx(() ==> $lz<$xz);Cx(() ==> $lz<=$xz);Cx(() ==> $lz>$xz);Cx(() ==> $lz>=$xz);Cx(() ==> $lz==$xz);Cx(() ==> $lz===$xz);
    Ix(() ==> $lz<=>$xz);
  begin_row('qp', 'WRAPD');
    Cx(() ==> $qz<$xz);Cx(() ==> $qz<=$xz);Cx(() ==> $qz>$xz);Cx(() ==> $qz>=$xz);Cx(() ==> $qz==$xz);Cx(() ==> $qz===$xz);
    Ix(() ==> $qz<=>$xz);
  print_footer();
}

<<__NEVER_INLINE>> function dynamic_compare() :mixed{
  $cm = LV(Foo::bar<>);
  $nv = LV(null);
  $tv = LV(true);
  $bv = LV(false);
  $iv = LV(42);
  $fv = LV(3.14159);
  $sv = LV('Foo::bar');
  $rv = LV(opendir(getcwd()));
  $ov = LV(new StrObj('Foo::bar'));
  $va = LV(vec[Foo::class, 'bar']);
  $da = LV(dict[0 => Foo::class, 1 => 'bar']);
  $cp = LV(Foo::bar<>);
  $ep = LV(bar<>);
  $lp = LV(Foo::baz<>);
  $qp = LV(CLS('Foo'));

  $xx = WRAPA($cm); $nx = WRAPA($nv); $tx = WRAPA($tv); $bx = WRAPA($bv);
  $ix = WRAPA($iv); $fx = WRAPA($fv); $sx = WRAPA($sv); $rx = WRAPA($rv);
  $ox = WRAPA($ov); $vx = WRAPA($va); $dx = WRAPA($da); $cx = WRAPA($cp);
  $ex = WRAPA($ep); $lx = WRAPA($lp); $qx = WRAPA($qp);

  $xy = WRAPO($cm); $ny = WRAPO($nv); $ty = WRAPO($tv); $by = WRAPO($bv);
  $iy = WRAPO($iv); $fy = WRAPO($fv); $sy = WRAPO($sv); $ry = WRAPO($rv);
  $oy = WRAPO($ov); $vy = WRAPO($va); $dy = WRAPO($da); $cy = WRAPO($cp);
  $ey = WRAPO($ep); $ly = WRAPO($lp); $qy = WRAPO($qp);

  $xz = WRAPD($cm); $nz = WRAPD($nv); $tz = WRAPD($tv); $bz = WRAPD($bv);
  $iz = WRAPD($iv); $fz = WRAPD($fv); $sz = WRAPD($sv); $rz = WRAPD($rv);
  $oz = WRAPD($ov); $vz = WRAPD($va); $dz = WRAPD($da); $cz = WRAPD($cp);
  $ez = WRAPD($ep); $lz = WRAPD($lp); $qz = WRAPD($qp);

  print_header('[dynamic] $cm ? VAR');
  begin_row('cm');
    Cx(() ==> $cm<$cm);Cx(() ==> $cm<=$cm);Cx(() ==> $cm>$cm);Cx(() ==> $cm>=$cm);Cx(() ==> $cm==$cm);Cx(() ==> $cm===$cm);
    Ix(() ==> $cm<=>$cm);
  begin_row('nv');
    Cx(() ==> $cm<$nv);Cx(() ==> $cm<=$nv);Cx(() ==> $cm>$nv);Cx(() ==> $cm>=$nv);Cx(() ==> $cm==$nv);Cx(() ==> $cm===$nv);
    Ix(() ==> $cm<=>$nv);
  begin_row('tv');
    Cx(() ==> $cm<$tv);Cx(() ==> $cm<=$tv);Cx(() ==> $cm>$tv);Cx(() ==> $cm>=$tv);Cx(() ==> $cm==$tv);Cx(() ==> $cm===$tv);
    Ix(() ==> $cm<=>$tv);
  begin_row('bv');
    Cx(() ==> $cm<$bv);Cx(() ==> $cm<=$bv);Cx(() ==> $cm>$bv);Cx(() ==> $cm>=$bv);Cx(() ==> $cm==$bv);Cx(() ==> $cm===$bv);
    Ix(() ==> $cm<=>$bv);
  begin_row('iv');
    Cx(() ==> $cm<$iv);Cx(() ==> $cm<=$iv);Cx(() ==> $cm>$iv);Cx(() ==> $cm>=$iv);Cx(() ==> $cm==$iv);Cx(() ==> $cm===$iv);
    Ix(() ==> $cm<=>$iv);
  begin_row('fv');
    Cx(() ==> $cm<$fv);Cx(() ==> $cm<=$fv);Cx(() ==> $cm>$fv);Cx(() ==> $cm>=$fv);Cx(() ==> $cm==$fv);Cx(() ==> $cm===$fv);
    Ix(() ==> $cm<=>$fv);
  begin_row('sv');
    Cx(() ==> $cm<$sv);Cx(() ==> $cm<=$sv);Cx(() ==> $cm>$sv);Cx(() ==> $cm>=$sv);Cx(() ==> $cm==$sv);Cx(() ==> $cm===$sv);
    Ix(() ==> $cm<=>$sv);
  begin_row('rv');
    Cx(() ==> $cm<$rv);Cx(() ==> $cm<=$rv);Cx(() ==> $cm>$rv);Cx(() ==> $cm>=$rv);Cx(() ==> $cm==$rv);Cx(() ==> $cm===$rv);
    Ix(() ==> $cm<=>$rv);
  begin_row('ov');
    Cx(() ==> $cm<$ov);Cx(() ==> $cm<=$ov);Cx(() ==> $cm>$ov);Cx(() ==> $cm>=$ov);Cx(() ==> $cm==$ov);Cx(() ==> $cm===$ov);
    Ix(() ==> $cm<=>$ov);
  begin_row('va');
    Cx(() ==> $cm<$va);Cx(() ==> $cm<=$va);Cx(() ==> $cm>$va);Cx(() ==> $cm>=$va);Cx(() ==> $cm==$va);Cx(() ==> $cm===$va);
    Ix(() ==> $cm<=>$va);
  begin_row('cp');
    Cx(() ==> $cm<$cp);Cx(() ==> $cm<=$cp);Cx(() ==> $cm>$cp);Cx(() ==> $cm>=$cp);Cx(() ==> $cm==$cp);Cx(() ==> $cm===$cp);
    Ix(() ==> $cm<=>$cp);
  begin_row('ep');
    Cx(() ==> $cm<$ep);Cx(() ==> $cm<=$ep);Cx(() ==> $cm>$ep);Cx(() ==> $cm>=$ep);Cx(() ==> $cm==$ep);Cx(() ==> $cm===$ep);
    Ix(() ==> $cm<=>$ep);
  begin_row('lp');
    Cx(() ==> $cm<$lp);Cx(() ==> $cm<=$lp);Cx(() ==> $cm>$lp);Cx(() ==> $cm>=$lp);Cx(() ==> $cm==$lp);Cx(() ==> $cm===$lp);
    Ix(() ==> $cm<=>$lp);
  begin_row('qp');
    Cx(() ==> $cm<$qp);Cx(() ==> $cm<=$qp);Cx(() ==> $cm>$qp);Cx(() ==> $cm>=$qp);Cx(() ==> $cm==$qp);Cx(() ==> $cm===$qp);
    Ix(() ==> $cm<=>$qp);

  begin_row('nv', 'WRAPA');
    Cx(() ==> $xx<$nx);Cx(() ==> $xx<=$nx);Cx(() ==> $xx>$nx);Cx(() ==> $xx>=$nx);Cx(() ==> $xx==$nx);Cx(() ==> $xx===$nx);
    Ix(() ==> $xx<=>$nx);
  begin_row('tv', 'WRAPA');
    Cx(() ==> $xx<$tx);Cx(() ==> $xx<=$tx);Cx(() ==> $xx>$tx);Cx(() ==> $xx>=$tx);Cx(() ==> $xx==$tx);Cx(() ==> $xx===$tx);
    Ix(() ==> $xx<=>$tx);
  begin_row('bv', 'WRAPA');
    Cx(() ==> $xx<$bx);Cx(() ==> $xx<=$bx);Cx(() ==> $xx>$bx);Cx(() ==> $xx>=$bx);Cx(() ==> $xx==$bx);Cx(() ==> $xx===$bx);
    Ix(() ==> $xx<=>$bx);
  begin_row('iv', 'WRAPA');
    Cx(() ==> $xx<$ix);Cx(() ==> $xx<=$ix);Cx(() ==> $xx>$ix);Cx(() ==> $xx>=$ix);Cx(() ==> $xx==$ix);Cx(() ==> $xx===$ix);
    Ix(() ==> $xx<=>$ix);
  begin_row('fv', 'WRAPA');
    Cx(() ==> $xx<$fx);Cx(() ==> $xx<=$fx);Cx(() ==> $xx>$fx);Cx(() ==> $xx>=$fx);Cx(() ==> $xx==$fx);Cx(() ==> $xx===$fx);
    Ix(() ==> $xx<=>$fx);
  begin_row('sv', 'WRAPA');
    Cx(() ==> $xx<$sx);Cx(() ==> $xx<=$sx);Cx(() ==> $xx>$sx);Cx(() ==> $xx>=$sx);Cx(() ==> $xx==$sx);Cx(() ==> $xx===$sx);
    Ix(() ==> $xx<=>$sx);
  begin_row('rv', 'WRAPA');
    Cx(() ==> $xx<$rx);Cx(() ==> $xx<=$rx);Cx(() ==> $xx>$rx);Cx(() ==> $xx>=$rx);Cx(() ==> $xx==$rx);Cx(() ==> $xx===$rx);
    Ix(() ==> $xx<=>$rx);
  begin_row('ov', 'WRAPA');
    Cx(() ==> $xx<$ox);Cx(() ==> $xx<=$ox);Cx(() ==> $xx>$ox);Cx(() ==> $xx>=$ox);Cx(() ==> $xx==$ox);Cx(() ==> $xx===$ox);
    Ix(() ==> $xx<=>$ox);
  begin_row('va', 'WRAPA');
    Cx(() ==> $xx<$vx);Cx(() ==> $xx<=$vx);Cx(() ==> $xx>$vx);Cx(() ==> $xx>=$vx);Cx(() ==> $xx==$vx);Cx(() ==> $xx===$vx);
    Ix(() ==> $xx<=>$vx);
  begin_row('cp', 'WRAPA');
    Cx(() ==> $xx<$cx);Cx(() ==> $xx<=$cx);Cx(() ==> $xx>$cx);Cx(() ==> $xx>=$cx);Cx(() ==> $xx==$cx);Cx(() ==> $xx===$cx);
    Ix(() ==> $xx<=>$cx);
  begin_row('ep', 'WRAPA');
    Cx(() ==> $xx<$ex);Cx(() ==> $xx<=$ex);Cx(() ==> $xx>$ex);Cx(() ==> $xx>=$ex);Cx(() ==> $xx==$ex);Cx(() ==> $xx===$ex);
    Ix(() ==> $xx<=>$ex);
  begin_row('lp', 'WRAPA');
    Cx(() ==> $xx<$lx);Cx(() ==> $xx<=$lx);Cx(() ==> $xx>$lx);Cx(() ==> $xx>=$lx);Cx(() ==> $xx==$lx);Cx(() ==> $xx===$lx);
    Ix(() ==> $xx<=>$lx);
  begin_row('qp', 'WRAPA');
    Cx(() ==> $xx<$qx);Cx(() ==> $xx<=$qx);Cx(() ==> $xx>$qx);Cx(() ==> $xx>=$qx);Cx(() ==> $xx==$qx);Cx(() ==> $xx===$qx);
    Ix(() ==> $xx<=>$qx);

  begin_row('nv', 'WRAPO');
    Cx(() ==> $xy<$ny);Cx(() ==> $xy<=$ny);Cx(() ==> $xy>$ny);Cx(() ==> $xy>=$ny);Cx(() ==> $xy==$ny);Cx(() ==> $xy===$ny);
    Ix(() ==> $xy<=>$ny);
  begin_row('tv', 'WRAPO');
    Cx(() ==> $xy<$ty);Cx(() ==> $xy<=$ty);Cx(() ==> $xy>$ty);Cx(() ==> $xy>=$ty);Cx(() ==> $xy==$ty);Cx(() ==> $xy===$ty);
    Ix(() ==> $xy<=>$ty);
  begin_row('bv', 'WRAPO');
    Cx(() ==> $xy<$by);Cx(() ==> $xy<=$by);Cx(() ==> $xy>$by);Cx(() ==> $xy>=$by);Cx(() ==> $xy==$by);Cx(() ==> $xy===$by);
    Ix(() ==> $xy<=>$by);
  begin_row('iv', 'WRAPO');
    Cx(() ==> $xy<$iy);Cx(() ==> $xy<=$iy);Cx(() ==> $xy>$iy);Cx(() ==> $xy>=$iy);Cx(() ==> $xy==$iy);Cx(() ==> $xy===$iy);
    Ix(() ==> $xy<=>$iy);
  begin_row('fv', 'WRAPO');
    Cx(() ==> $xy<$fy);Cx(() ==> $xy<=$fy);Cx(() ==> $xy>$fy);Cx(() ==> $xy>=$fy);Cx(() ==> $xy==$fy);Cx(() ==> $xy===$fy);
    Ix(() ==> $xy<=>$fy);
  begin_row('sv', 'WRAPO');
    Cx(() ==> $xy<$sy);Cx(() ==> $xy<=$sy);Cx(() ==> $xy>$sy);Cx(() ==> $xy>=$sy);Cx(() ==> $xy==$sy);Cx(() ==> $xy===$sy);
    Ix(() ==> $xy<=>$sy);
  begin_row('rv', 'WRAPO');
    Cx(() ==> $xy<$ry);Cx(() ==> $xy<=$ry);Cx(() ==> $xy>$ry);Cx(() ==> $xy>=$ry);Cx(() ==> $xy==$ry);Cx(() ==> $xy===$ry);
    Ix(() ==> $xy<=>$ry);
  begin_row('ov', 'WRAPO');
    Cx(() ==> $xy<$oy);Cx(() ==> $xy<=$oy);Cx(() ==> $xy>$oy);Cx(() ==> $xy>=$oy);Cx(() ==> $xy==$oy);Cx(() ==> $xy===$oy);
    Ix(() ==> $xy<=>$oy);
  begin_row('va', 'WRAPO');
    Cx(() ==> $xy<$vy);Cx(() ==> $xy<=$vy);Cx(() ==> $xy>$vy);Cx(() ==> $xy>=$vy);Cx(() ==> $xy==$vy);Cx(() ==> $xy===$vy);
    Ix(() ==> $xy<=>$vy);
  begin_row('cp', 'WRAPO');
    Cx(() ==> $xy<$cy);Cx(() ==> $xy<=$cy);Cx(() ==> $xy>$cy);Cx(() ==> $xy>=$cy);Cx(() ==> $xy==$cy);Cx(() ==> $xy===$cy);
    Ix(() ==> $xy<=>$cy);
  begin_row('ep', 'WRAPO');
    Cx(() ==> $xy<$ey);Cx(() ==> $xy<=$ey);Cx(() ==> $xy>$ey);Cx(() ==> $xy>=$ey);Cx(() ==> $xy==$ey);Cx(() ==> $xy===$ey);
    Ix(() ==> $xy<=>$ey);
  begin_row('lp', 'WRAPO');
    Cx(() ==> $xy<$ly);Cx(() ==> $xy<=$ly);Cx(() ==> $xy>$ly);Cx(() ==> $xy>=$ly);Cx(() ==> $xy==$ly);Cx(() ==> $xy===$ly);
    Ix(() ==> $xy<=>$ly);
  begin_row('qp', 'WRAPO');
    Cx(() ==> $xy<$qy);Cx(() ==> $xy<=$qy);Cx(() ==> $xy>$qy);Cx(() ==> $xy>=$qy);Cx(() ==> $xy==$qy);Cx(() ==> $xy===$qy);
    Ix(() ==> $xy<=>$qy);

  begin_row('nv', 'WRAPD');
    Cx(() ==> $xz<$nz);Cx(() ==> $xz<=$nz);Cx(() ==> $xz>$nz);Cx(() ==> $xz>=$nz);Cx(() ==> $xz==$nz);Cx(() ==> $xz===$nz);
    Ix(() ==> $xz<=>$nz);
  begin_row('tv', 'WRAPD');
    Cx(() ==> $xz<$tz);Cx(() ==> $xz<=$tz);Cx(() ==> $xz>$tz);Cx(() ==> $xz>=$tz);Cx(() ==> $xz==$tz);Cx(() ==> $xz===$tz);
    Ix(() ==> $xz<=>$tz);
  begin_row('bv', 'WRAPD');
    Cx(() ==> $xz<$bz);Cx(() ==> $xz<=$bz);Cx(() ==> $xz>$bz);Cx(() ==> $xz>=$bz);Cx(() ==> $xz==$bz);Cx(() ==> $xz===$bz);
    Ix(() ==> $xz<=>$bz);
  begin_row('iv', 'WRAPD');
    Cx(() ==> $xz<$iz);Cx(() ==> $xz<=$iz);Cx(() ==> $xz>$iz);Cx(() ==> $xz>=$iz);Cx(() ==> $xz==$iz);Cx(() ==> $xz===$iz);
    Ix(() ==> $xz<=>$iz);
  begin_row('fv', 'WRAPD');
    Cx(() ==> $xz<$fz);Cx(() ==> $xz<=$fz);Cx(() ==> $xz>$fz);Cx(() ==> $xz>=$fz);Cx(() ==> $xz==$fz);Cx(() ==> $xz===$fz);
    Ix(() ==> $xz<=>$fz);
  begin_row('sv', 'WRAPD');
    Cx(() ==> $xz<$sz);Cx(() ==> $xz<=$sz);Cx(() ==> $xz>$sz);Cx(() ==> $xz>=$sz);Cx(() ==> $xz==$sz);Cx(() ==> $xz===$sz);
    Ix(() ==> $xz<=>$sz);
  begin_row('rv', 'WRAPD');
    Cx(() ==> $xz<$rz);Cx(() ==> $xz<=$rz);Cx(() ==> $xz>$rz);Cx(() ==> $xz>=$rz);Cx(() ==> $xz==$rz);Cx(() ==> $xz===$rz);
    Ix(() ==> $xz<=>$rz);
  begin_row('ov', 'WRAPD');
    Cx(() ==> $xz<$oz);Cx(() ==> $xz<=$oz);Cx(() ==> $xz>$oz);Cx(() ==> $xz>=$oz);Cx(() ==> $xz==$oz);Cx(() ==> $xz===$oz);
    Ix(() ==> $xz<=>$oz);
  begin_row('va', 'WRAPD');
    Cx(() ==> $xz<$vz);Cx(() ==> $xz<=$vz);Cx(() ==> $xz>$vz);Cx(() ==> $xz>=$vz);Cx(() ==> $xz==$vz);Cx(() ==> $xz===$vz);
    Ix(() ==> $xz<=>$vz);
  begin_row('cp', 'WRAPD');
    Cx(() ==> $xz<$cz);Cx(() ==> $xz<=$cz);Cx(() ==> $xz>$cz);Cx(() ==> $xz>=$cz);Cx(() ==> $xz==$cz);Cx(() ==> $xz===$cz);
    Ix(() ==> $xz<=>$cz);
  begin_row('ep', 'WRAPD');
    Cx(() ==> $xz<$ez);Cx(() ==> $xz<=$ez);Cx(() ==> $xz>$ez);Cx(() ==> $xz>=$ez);Cx(() ==> $xz==$ez);Cx(() ==> $xz===$ez);
    Ix(() ==> $xz<=>$ez);
  begin_row('lp', 'WRAPD');
    Cx(() ==> $xz<$lz);Cx(() ==> $xz<=$lz);Cx(() ==> $xz>$lz);Cx(() ==> $xz>=$lz);Cx(() ==> $xz==$lz);Cx(() ==> $xz===$lz);
    Ix(() ==> $xz<=>$lz);
  begin_row('qp', 'WRAPD');
    Cx(() ==> $xz<$qz);Cx(() ==> $xz<=$qz);Cx(() ==> $xz>$qz);Cx(() ==> $xz>=$qz);Cx(() ==> $xz==$qz);Cx(() ==> $xz===$qz);
    Ix(() ==> $xz<=>$qz);
  print_footer();

  print_header('[dynamic] VAR ? $cm');
  begin_row('cm');
    Cx(() ==> $cm<$cm);Cx(() ==> $cm<=$cm);Cx(() ==> $cm>$cm);Cx(() ==> $cm>=$cm);Cx(() ==> $cm==$cm);Cx(() ==> $cm===$cm);
    Ix(() ==> $cm<=>$cm);
  begin_row('nv');
    Cx(() ==> $nv<$cm);Cx(() ==> $nv<=$cm);Cx(() ==> $nv>$cm);Cx(() ==> $nv>=$cm);Cx(() ==> $nv==$cm);Cx(() ==> $nv===$cm);
    Ix(() ==> $nv<=>$cm);
  begin_row('tv');
    Cx(() ==> $tv<$cm);Cx(() ==> $tv<=$cm);Cx(() ==> $tv>$cm);Cx(() ==> $tv>=$cm);Cx(() ==> $tv==$cm);Cx(() ==> $tv===$cm);
    Ix(() ==> $tv<=>$cm);
  begin_row('bv');
    Cx(() ==> $bv<$cm);Cx(() ==> $bv<=$cm);Cx(() ==> $bv>$cm);Cx(() ==> $bv>=$cm);Cx(() ==> $bv==$cm);Cx(() ==> $bv===$cm);
    Ix(() ==> $bv<=>$cm);
  begin_row('iv');
    Cx(() ==> $iv<$cm);Cx(() ==> $iv<=$cm);Cx(() ==> $iv>$cm);Cx(() ==> $iv>=$cm);Cx(() ==> $iv==$cm);Cx(() ==> $iv===$cm);
    Ix(() ==> $iv<=>$cm);
  begin_row('fv');
    Cx(() ==> $fv<$cm);Cx(() ==> $fv<=$cm);Cx(() ==> $fv>$cm);Cx(() ==> $fv>=$cm);Cx(() ==> $fv==$cm);Cx(() ==> $fv===$cm);
    Ix(() ==> $fv<=>$cm);
  begin_row('sv');
    Cx(() ==> $sv<$cm);Cx(() ==> $sv<=$cm);Cx(() ==> $sv>$cm);Cx(() ==> $sv>=$cm);Cx(() ==> $sv==$cm);Cx(() ==> $sv===$cm);
    Ix(() ==> $sv<=>$cm);
  begin_row('rv');
    Cx(() ==> $rv<$cm);Cx(() ==> $rv<=$cm);Cx(() ==> $rv>$cm);Cx(() ==> $rv>=$cm);Cx(() ==> $rv==$cm);Cx(() ==> $rv===$cm);
    Ix(() ==> $rv<=>$cm);
  begin_row('ov');
    Cx(() ==> $ov<$cm);Cx(() ==> $ov<=$cm);Cx(() ==> $ov>$cm);Cx(() ==> $ov>=$cm);Cx(() ==> $ov==$cm);Cx(() ==> $ov===$cm);
    Ix(() ==> $ov<=>$cm);
  begin_row('va');
    Cx(() ==> $va<$cm);Cx(() ==> $va<=$cm);Cx(() ==> $va>$cm);Cx(() ==> $va>=$cm);Cx(() ==> $va==$cm);Cx(() ==> $va===$cm);
    Ix(() ==> $va<=>$cm);
  begin_row('cp');
    Cx(() ==> $cp<$cm);Cx(() ==> $cp<=$cm);Cx(() ==> $cp>$cm);Cx(() ==> $cp>=$cm);Cx(() ==> $cp==$cm);Cx(() ==> $cp===$cm);
    Ix(() ==> $cp<=>$cm);
  begin_row('ep');
    Cx(() ==> $ep<$cm);Cx(() ==> $ep<=$cm);Cx(() ==> $ep>$cm);Cx(() ==> $ep>=$cm);Cx(() ==> $ep==$cm);Cx(() ==> $ep===$cm);
    Ix(() ==> $ep<=>$cm);
  begin_row('lp');
    Cx(() ==> $lp<$cm);Cx(() ==> $lp<=$cm);Cx(() ==> $lp>$cm);Cx(() ==> $lp>=$cm);Cx(() ==> $lp==$cm);Cx(() ==> $lp===$cm);
    Ix(() ==> $lp<=>$cm);
  begin_row('qp');
    Cx(() ==> $qp<$cm);Cx(() ==> $qp<=$cm);Cx(() ==> $qp>$cm);Cx(() ==> $qp>=$cm);Cx(() ==> $qp==$cm);Cx(() ==> $qp===$cm);
    Ix(() ==> $qp<=>$cm);

  begin_row('nv', 'WRAPA');
    Cx(() ==> $nx<$xx);Cx(() ==> $nx<=$xx);Cx(() ==> $nx>$xx);Cx(() ==> $nx>=$xx);Cx(() ==> $nx==$xx);Cx(() ==> $nx===$xx);
    Ix(() ==> $nx<=>$xx);
  begin_row('tv', 'WRAPA');
    Cx(() ==> $tx<$xx);Cx(() ==> $tx<=$xx);Cx(() ==> $tx>$xx);Cx(() ==> $tx>=$xx);Cx(() ==> $tx==$xx);Cx(() ==> $tx===$xx);
    Ix(() ==> $tx<=>$xx);
  begin_row('bv', 'WRAPA');
    Cx(() ==> $bx<$xx);Cx(() ==> $bx<=$xx);Cx(() ==> $bx>$xx);Cx(() ==> $bx>=$xx);Cx(() ==> $bx==$xx);Cx(() ==> $bx===$xx);
    Ix(() ==> $bx<=>$xx);
  begin_row('iv', 'WRAPA');
    Cx(() ==> $ix<$xx);Cx(() ==> $ix<=$xx);Cx(() ==> $ix>$xx);Cx(() ==> $ix>=$xx);Cx(() ==> $ix==$xx);Cx(() ==> $ix===$xx);
    Ix(() ==> $ix<=>$xx);
  begin_row('fv', 'WRAPA');
    Cx(() ==> $fx<$xx);Cx(() ==> $fx<=$xx);Cx(() ==> $fx>$xx);Cx(() ==> $fx>=$xx);Cx(() ==> $fx==$xx);Cx(() ==> $fx===$xx);
    Ix(() ==> $fx<=>$xx);
  begin_row('sv', 'WRAPA');
    Cx(() ==> $sx<$xx);Cx(() ==> $sx<=$xx);Cx(() ==> $sx>$xx);Cx(() ==> $sx>=$xx);Cx(() ==> $sx==$xx);Cx(() ==> $sx===$xx);
    Ix(() ==> $sx<=>$xx);
  begin_row('rv', 'WRAPA');
    Cx(() ==> $rx<$xx);Cx(() ==> $rx<=$xx);Cx(() ==> $rx>$xx);Cx(() ==> $rx>=$xx);Cx(() ==> $rx==$xx);Cx(() ==> $rx===$xx);
    Ix(() ==> $rx<=>$xx);
  begin_row('ov', 'WRAPA');
    Cx(() ==> $ox<$xx);Cx(() ==> $ox<=$xx);Cx(() ==> $ox>$xx);Cx(() ==> $ox>=$xx);Cx(() ==> $ox==$xx);Cx(() ==> $ox===$xx);
    Ix(() ==> $ox<=>$xx);
  begin_row('va', 'WRAPA');
    Cx(() ==> $vx<$xx);Cx(() ==> $vx<=$xx);Cx(() ==> $vx>$xx);Cx(() ==> $vx>=$xx);Cx(() ==> $vx==$xx);Cx(() ==> $vx===$xx);
    Ix(() ==> $vx<=>$xx);
  begin_row('cp', 'WRAPA');
    Cx(() ==> $cx<$xx);Cx(() ==> $cx<=$xx);Cx(() ==> $cx>$xx);Cx(() ==> $cx>=$xx);Cx(() ==> $cx==$xx);Cx(() ==> $cx===$xx);
    Ix(() ==> $cx<=>$xx);
  begin_row('ep', 'WRAPA');
    Cx(() ==> $ex<$xx);Cx(() ==> $ex<=$xx);Cx(() ==> $ex>$xx);Cx(() ==> $ex>=$xx);Cx(() ==> $ex==$xx);Cx(() ==> $ex===$xx);
    Ix(() ==> $ex<=>$xx);
  begin_row('lp', 'WRAPA');
    Cx(() ==> $lx<$xx);Cx(() ==> $lx<=$xx);Cx(() ==> $lx>$xx);Cx(() ==> $lx>=$xx);Cx(() ==> $lx==$xx);Cx(() ==> $lx===$xx);
    Ix(() ==> $lx<=>$xx);
  begin_row('qp', 'WRAPA');
    Cx(() ==> $qx<$xx);Cx(() ==> $qx<=$xx);Cx(() ==> $qx>$xx);Cx(() ==> $qx>=$xx);Cx(() ==> $qx==$xx);Cx(() ==> $qx===$xx);
    Ix(() ==> $qx<=>$xx);

  begin_row('nv', 'WRAPO');
    Cx(() ==> $ny<$xy);Cx(() ==> $ny<=$xy);Cx(() ==> $ny>$xy);Cx(() ==> $ny>=$xy);Cx(() ==> $ny==$xy);Cx(() ==> $ny===$xy);
    Ix(() ==> $ny<=>$xy);
  begin_row('tv', 'WRAPO');
    Cx(() ==> $ty<$xy);Cx(() ==> $ty<=$xy);Cx(() ==> $ty>$xy);Cx(() ==> $ty>=$xy);Cx(() ==> $ty==$xy);Cx(() ==> $ty===$xy);
    Ix(() ==> $ty<=>$xy);
  begin_row('bv', 'WRAPO');
    Cx(() ==> $by<$xy);Cx(() ==> $by<=$xy);Cx(() ==> $by>$xy);Cx(() ==> $by>=$xy);Cx(() ==> $by==$xy);Cx(() ==> $by===$xy);
    Ix(() ==> $by<=>$xy);
  begin_row('iv', 'WRAPO');
    Cx(() ==> $iy<$xy);Cx(() ==> $iy<=$xy);Cx(() ==> $iy>$xy);Cx(() ==> $iy>=$xy);Cx(() ==> $iy==$xy);Cx(() ==> $iy===$xy);
    Ix(() ==> $iy<=>$xy);
  begin_row('fv', 'WRAPO');
    Cx(() ==> $fy<$xy);Cx(() ==> $fy<=$xy);Cx(() ==> $fy>$xy);Cx(() ==> $fy>=$xy);Cx(() ==> $fy==$xy);Cx(() ==> $fy===$xy);
    Ix(() ==> $fy<=>$xy);
  begin_row('sv', 'WRAPO');
    Cx(() ==> $sy<$xy);Cx(() ==> $sy<=$xy);Cx(() ==> $sy>$xy);Cx(() ==> $sy>=$xy);Cx(() ==> $sy==$xy);Cx(() ==> $sy===$xy);
    Ix(() ==> $sy<=>$xy);
  begin_row('rv', 'WRAPO');
    Cx(() ==> $ry<$xy);Cx(() ==> $ry<=$xy);Cx(() ==> $ry>$xy);Cx(() ==> $ry>=$xy);Cx(() ==> $ry==$xy);Cx(() ==> $ry===$xy);
    Ix(() ==> $ry<=>$xy);
  begin_row('ov', 'WRAPO');
    Cx(() ==> $oy<$xy);Cx(() ==> $oy<=$xy);Cx(() ==> $oy>$xy);Cx(() ==> $oy>=$xy);Cx(() ==> $oy==$xy);Cx(() ==> $oy===$xy);
    Ix(() ==> $oy<=>$xy);
  begin_row('va', 'WRAPO');
    Cx(() ==> $vy<$xy);Cx(() ==> $vy<=$xy);Cx(() ==> $vy>$xy);Cx(() ==> $vy>=$xy);Cx(() ==> $vy==$xy);Cx(() ==> $vy===$xy);
    Ix(() ==> $vy<=>$xy);
  begin_row('cp', 'WRAPO');
    Cx(() ==> $cy<$xy);Cx(() ==> $cy<=$xy);Cx(() ==> $cy>$xy);Cx(() ==> $cy>=$xy);Cx(() ==> $cy==$xy);Cx(() ==> $cy===$xy);
    Ix(() ==> $cy<=>$xy);
  begin_row('ep', 'WRAPO');
    Cx(() ==> $ey<$xy);Cx(() ==> $ey<=$xy);Cx(() ==> $ey>$xy);Cx(() ==> $ey>=$xy);Cx(() ==> $ey==$xy);Cx(() ==> $ey===$xy);
    Ix(() ==> $ey<=>$xy);
  begin_row('lp', 'WRAPO');
    Cx(() ==> $ly<$xy);Cx(() ==> $ly<=$xy);Cx(() ==> $ly>$xy);Cx(() ==> $ly>=$xy);Cx(() ==> $ly==$xy);Cx(() ==> $ly===$xy);
    Ix(() ==> $ly<=>$xy);
  begin_row('qp', 'WRAPO');
    Cx(() ==> $qy<$xy);Cx(() ==> $qy<=$xy);Cx(() ==> $qy>$xy);Cx(() ==> $qy>=$xy);Cx(() ==> $qy==$xy);Cx(() ==> $qy===$xy);
    Ix(() ==> $qy<=>$xy);

  begin_row('nv', 'WRAPD');
    Cx(() ==> $nz<$xz);Cx(() ==> $nz<=$xz);Cx(() ==> $nz>$xz);Cx(() ==> $nz>=$xz);Cx(() ==> $nz==$xz);Cx(() ==> $nz===$xz);
    Ix(() ==> $nz<=>$xz);
  begin_row('tv', 'WRAPD');
    Cx(() ==> $tz<$xz);Cx(() ==> $tz<=$xz);Cx(() ==> $tz>$xz);Cx(() ==> $tz>=$xz);Cx(() ==> $tz==$xz);Cx(() ==> $tz===$xz);
    Ix(() ==> $tz<=>$xz);
  begin_row('bv', 'WRAPD');
    Cx(() ==> $bz<$xz);Cx(() ==> $bz<=$xz);Cx(() ==> $bz>$xz);Cx(() ==> $bz>=$xz);Cx(() ==> $bz==$xz);Cx(() ==> $bz===$xz);
    Ix(() ==> $bz<=>$xz);
  begin_row('iv', 'WRAPD');
    Cx(() ==> $iz<$xz);Cx(() ==> $iz<=$xz);Cx(() ==> $iz>$xz);Cx(() ==> $iz>=$xz);Cx(() ==> $iz==$xz);Cx(() ==> $iz===$xz);
    Ix(() ==> $iz<=>$xz);
  begin_row('fv', 'WRAPD');
    Cx(() ==> $fz<$xz);Cx(() ==> $fz<=$xz);Cx(() ==> $fz>$xz);Cx(() ==> $fz>=$xz);Cx(() ==> $fz==$xz);Cx(() ==> $fz===$xz);
    Ix(() ==> $fz<=>$xz);
  begin_row('sv', 'WRAPD');
    Cx(() ==> $sz<$xz);Cx(() ==> $sz<=$xz);Cx(() ==> $sz>$xz);Cx(() ==> $sz>=$xz);Cx(() ==> $sz==$xz);Cx(() ==> $sz===$xz);
    Ix(() ==> $sz<=>$xz);
  begin_row('rv', 'WRAPD');
    Cx(() ==> $rz<$xz);Cx(() ==> $rz<=$xz);Cx(() ==> $rz>$xz);Cx(() ==> $rz>=$xz);Cx(() ==> $rz==$xz);Cx(() ==> $rz===$xz);
    Ix(() ==> $rz<=>$xz);
  begin_row('ov', 'WRAPD');
    Cx(() ==> $oz<$xz);Cx(() ==> $oz<=$xz);Cx(() ==> $oz>$xz);Cx(() ==> $oz>=$xz);Cx(() ==> $oz==$xz);Cx(() ==> $oz===$xz);
    Ix(() ==> $oz<=>$xz);
  begin_row('va', 'WRAPD');
    Cx(() ==> $vz<$xz);Cx(() ==> $vz<=$xz);Cx(() ==> $vz>$xz);Cx(() ==> $vz>=$xz);Cx(() ==> $vz==$xz);Cx(() ==> $vz===$xz);
    Ix(() ==> $vz<=>$xz);
  begin_row('cp', 'WRAPD');
    Cx(() ==> $cz<$xz);Cx(() ==> $cz<=$xz);Cx(() ==> $cz>$xz);Cx(() ==> $cz>=$xz);Cx(() ==> $cz==$xz);Cx(() ==> $cz===$xz);
    Ix(() ==> $cz<=>$xz);
  begin_row('ep', 'WRAPD');
    Cx(() ==> $ez<$xz);Cx(() ==> $ez<=$xz);Cx(() ==> $ez>$xz);Cx(() ==> $ez>=$xz);Cx(() ==> $ez==$xz);Cx(() ==> $ez===$xz);
    Ix(() ==> $ez<=>$xz);
  begin_row('lp', 'WRAPD');
    Cx(() ==> $lz<$xz);Cx(() ==> $lz<=$xz);Cx(() ==> $lz>$xz);Cx(() ==> $lz>=$xz);Cx(() ==> $lz==$xz);Cx(() ==> $lz===$xz);
    Ix(() ==> $lz<=>$xz);
  begin_row('qp', 'WRAPD');
    Cx(() ==> $qz<$xz);Cx(() ==> $qz<=$xz);Cx(() ==> $qz>$xz);Cx(() ==> $qz>=$xz);Cx(() ==> $qz==$xz);Cx(() ==> $qz===$xz);
    Ix(() ==> $qz<=>$xz);
  print_footer();
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(handle_error<>);
  static_compare();
  dynamic_compare();
}
