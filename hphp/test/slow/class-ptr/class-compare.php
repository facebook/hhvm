<?hh

<<__DynamicallyReferenced>> class foobar {}
function foobar() :mixed{}
class StrObj {
  public function __construct(private string $s)[] {}
  public function __toString()[]: string { return $this->s; }
}
class Wrapper { public function __construct(private mixed $w)[] {} }

function LV($x)  :mixed{ return __hhvm_intrinsics\launder_value($x); }
function CLS($c) :mixed{ return HH\classname_to_class($c); }

function vLV($x) :mixed{ return LV(vec[$x]); }
function wLV($x) :mixed{ return LV(new Wrapper($x)); }
function pLV($x) :mixed{ $r = new stdClass; $r->x = $x; return LV($r); }

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
  printf(" %s    |", $v ? 'T' : 'F');
}
<<__NEVER_INLINE>> function print_divider() :mixed{
  echo "\n+------------+------+------+------+------+------+------+";
}
<<__NEVER_INLINE>> function print_footer() :mixed{
  echo "\n+------------+------+------+------+------+------+------+\n\n";
}

<<__NEVER_INLINE>> function static_compare() :mixed{
  // lcl for lazy class, using 3 chars instead of lcls so the cases align better
  $lcl = foobar::class;
  $str = 'foobar';
  $obj = new StrObj('foobar');
  $cls = CLS('foobar');

  // v prefix for vec
  $vlcl = vec[$lcl];
  $vstr = vec[$str];
  $vobj = vec[$obj];
  $vcls = vec[$cls];

  // w prefix for Wrapper
  $wlcl = new Wrapper($lcl);
  $wstr = new Wrapper($str);
  $wobj = new Wrapper($obj);
  $wcls = new Wrapper($cls);

  // p prefix for property
  $plcl = new stdClass; $plcl->v = $lcl;
  $pstr = new stdClass; $pstr->v = $str;
  $pobj = new stdClass; $pobj->v = $obj;
  $pcls = new stdClass; $pcls->v = $cls;

  // Note: we're not doing a total comparison here -- the wrap lines always
  // compare wrap-to-wrap.

  //// $lcl

  print_header('[static] $lcl ? VAR');
  begin_row('lcl');
    C($lcl<$lcl);C($lcl<=$lcl);C($lcl>$lcl);C($lcl>=$lcl);C($lcl==$lcl);C($lcl===$lcl);
  begin_row('str');
    C($lcl<$str);C($lcl<=$str);C($lcl>$str);C($lcl>=$str);C($lcl==$str);C($lcl===$str);
  begin_row('cls');
    C($lcl<$cls);C($lcl<=$cls);C($lcl>$cls);C($lcl>=$cls);C($lcl==$cls);C($lcl===$cls);
  begin_row('obj');
    C(HH\Lib\Legacy_FIXME\lt($lcl, $obj));C(HH\Lib\Legacy_FIXME\lte($lcl, $obj));C(HH\Lib\Legacy_FIXME\gt($lcl, $obj));C(HH\Lib\Legacy_FIXME\gte($lcl, $obj));C(HH\Lib\Legacy_FIXME\eq($lcl, $obj));C($lcl===$obj);
  print_divider();

  begin_row('lcl', 'v');
    C($vlcl<$vlcl);C($vlcl<=$vlcl);C($vlcl>$vlcl);C($vlcl>=$vlcl);C($vlcl==$vlcl);C($vlcl===$vlcl);
  begin_row('str', 'v');
    C($vlcl<$vstr);C($vlcl<=$vstr);C($vlcl>$vstr);C($vlcl>=$vstr);C($vlcl==$vstr);C($vlcl===$vstr);
  begin_row('cls', 'v');
    C($vlcl<$vcls);C($vlcl<=$vcls);C($vlcl>$vcls);C($vlcl>=$vcls);C($vlcl==$vcls);C($vlcl===$vcls);
  begin_row('obj', 'v');
    C(HH\Lib\Legacy_FIXME\lt($vlcl, $vobj));C(HH\Lib\Legacy_FIXME\lte($vlcl, $vobj));C(HH\Lib\Legacy_FIXME\gt($vlcl, $vobj));C(HH\Lib\Legacy_FIXME\gte($vlcl, $vobj));C(HH\Lib\Legacy_FIXME\eq($vlcl, $vobj));C($vlcl===$vobj);
  print_divider();

  begin_row('lcl', 'w');
    C($wlcl<$wlcl);C($wlcl<=$wlcl);C($wlcl>$wlcl);C($wlcl>=$wlcl);C($wlcl==$wlcl);C($wlcl===$wlcl);
  begin_row('str', 'w');
    C($wlcl<$wstr);C($wlcl<=$wstr);C($wlcl>$wstr);C($wlcl>=$wstr);C($wlcl==$wstr);C($wlcl===$wstr);
  begin_row('cls', 'w');
    C($wlcl<$wcls);C($wlcl<=$wcls);C($wlcl>$wcls);C($wlcl>=$wcls);C($wlcl==$wcls);C($wlcl===$wcls);
  begin_row('obj', 'w');
    C(HH\Lib\Legacy_FIXME\lt($wlcl, $wobj));C(HH\Lib\Legacy_FIXME\lte($wlcl, $wobj));C(HH\Lib\Legacy_FIXME\gt($wlcl, $wobj));C(HH\Lib\Legacy_FIXME\gte($wlcl, $wobj));C(HH\Lib\Legacy_FIXME\eq($wlcl, $wobj));C($wlcl===$wobj);
  print_divider();

  begin_row('lcl', 'p');
    C($plcl<$plcl);C($plcl<=$plcl);C($plcl>$plcl);C($plcl>=$plcl);C($plcl==$plcl);C($plcl===$plcl);
  begin_row('str', 'p');
    C($plcl<$pstr);C($plcl<=$pstr);C($plcl>$pstr);C($plcl>=$pstr);C($plcl==$pstr);C($plcl===$pstr);
  begin_row('cls', 'p');
    C($plcl<$pcls);C($plcl<=$pcls);C($plcl>$pcls);C($plcl>=$pcls);C($plcl==$pcls);C($plcl===$pcls);
  begin_row('obj', 'p');
    C(HH\Lib\Legacy_FIXME\lt($plcl, $pobj));C(HH\Lib\Legacy_FIXME\lte($plcl, $pobj));C(HH\Lib\Legacy_FIXME\gt($plcl, $pobj));C(HH\Lib\Legacy_FIXME\gte($plcl, $pobj));C(HH\Lib\Legacy_FIXME\eq($plcl, $pobj));C($plcl===$pobj);
  print_footer();

  print_header('[static] VAR ? $lcl');
  begin_row('lcl');
    C($lcl<$lcl);C($lcl<=$lcl);C($lcl>$lcl);C($lcl>=$lcl);C($lcl==$lcl);C($lcl===$lcl);
  begin_row('str');
    C($str<$lcl);C($str<=$lcl);C($str>$lcl);C($str>=$lcl);C($str==$lcl);C($str===$lcl);
  begin_row('cls');
    C($cls<$lcl);C($cls<=$lcl);C($cls>$lcl);C($cls>=$lcl);C($cls==$lcl);C($cls===$lcl);
  begin_row('obj');
    C(HH\Lib\Legacy_FIXME\lt($obj, $lcl));C(HH\Lib\Legacy_FIXME\lte($obj, $lcl));C(HH\Lib\Legacy_FIXME\gt($obj, $lcl));C(HH\Lib\Legacy_FIXME\gte($obj, $lcl));C(HH\Lib\Legacy_FIXME\eq($obj, $lcl));C($obj===$lcl);
  print_divider();

  begin_row('lcl', 'v');
    C($vlcl<$vlcl);C($vlcl<=$vlcl);C($vlcl>$vlcl);C($vlcl>=$vlcl);C($vlcl==$vlcl);C($vlcl===$vlcl);
  begin_row('str', 'v');
    C($vstr<$vlcl);C($vstr<=$vlcl);C($vstr>$vlcl);C($vstr>=$vlcl);C($vstr==$vlcl);C($vstr===$vlcl);
  begin_row('cls', 'v');
    C($vcls<$vlcl);C($vcls<=$vlcl);C($vcls>$vlcl);C($vcls>=$vlcl);C($vcls==$vlcl);C($vcls===$vlcl);
  begin_row('obj', 'v');
    C(HH\Lib\Legacy_FIXME\lt($vobj, $vlcl));C(HH\Lib\Legacy_FIXME\lte($vobj, $vlcl));C(HH\Lib\Legacy_FIXME\gt($vobj, $vlcl));C(HH\Lib\Legacy_FIXME\gte($vobj, $vlcl));C(HH\Lib\Legacy_FIXME\eq($vobj, $vlcl));C($vobj===$vlcl);
  print_divider();

  begin_row('lcl', 'w');
    C($wlcl<$wlcl);C($wlcl<=$wlcl);C($wlcl>$wlcl);C($wlcl>=$wlcl);C($wlcl==$wlcl);C($wlcl===$wlcl);
  begin_row('str', 'w');
    C($wstr<$wlcl);C($wstr<=$wlcl);C($wstr>$wlcl);C($wstr>=$wlcl);C($wstr==$wlcl);C($wstr===$wlcl);
  begin_row('cls', 'w');
    C($wcls<$wlcl);C($wcls<=$wlcl);C($wcls>$wlcl);C($wcls>=$wlcl);C($wcls==$wlcl);C($wcls===$wlcl);
  begin_row('obj', 'w');
    C(HH\Lib\Legacy_FIXME\lt($wobj, $wlcl));C(HH\Lib\Legacy_FIXME\lte($wobj, $wlcl));C(HH\Lib\Legacy_FIXME\gt($wobj, $wlcl));C(HH\Lib\Legacy_FIXME\gte($wobj, $wlcl));C(HH\Lib\Legacy_FIXME\eq($wobj, $wlcl));C($wobj===$wlcl);
  print_divider();

  begin_row('lcl', 'p');
    C($plcl<$plcl);C($plcl<=$plcl);C($plcl>$plcl);C($plcl>=$plcl);C($plcl==$plcl);C($plcl===$plcl);
  begin_row('str', 'p');
    C($pstr<$plcl);C($pstr<=$plcl);C($pstr>$plcl);C($pstr>=$plcl);C($pstr==$plcl);C($pstr===$plcl);
  begin_row('cls', 'p');
    C($pcls<$plcl);C($pcls<=$plcl);C($pcls>$plcl);C($pcls>=$plcl);C($pcls==$plcl);C($pcls===$plcl);
  begin_row('obj', 'p');
    C(HH\Lib\Legacy_FIXME\lt($pobj, $plcl));C(HH\Lib\Legacy_FIXME\lte($pobj, $plcl));C(HH\Lib\Legacy_FIXME\gt($pobj, $plcl));C(HH\Lib\Legacy_FIXME\gte($pobj, $plcl));C(HH\Lib\Legacy_FIXME\eq($pobj, $plcl));C($pobj===$plcl);
  print_footer();

  //// $cls

  print_header('[static] $cls ? VAR');
  begin_row('lcl');
    C($cls<$lcl);C($cls<=$lcl);C($cls>$lcl);C($cls>=$lcl);C($cls==$lcl);C($cls===$lcl);
  begin_row('str');
    C($cls<$str);C($cls<=$str);C($cls>$str);C($cls>=$str);C($cls==$str);C($cls===$str);
  begin_row('cls');
    C($cls<$cls);C($cls<=$cls);C($cls>$cls);C($cls>=$cls);C($cls==$cls);C($cls===$cls);
  begin_row('obj');
    C(HH\Lib\Legacy_FIXME\lt($cls, $obj));C(HH\Lib\Legacy_FIXME\lte($cls, $obj));C(HH\Lib\Legacy_FIXME\gt($cls, $obj));C(HH\Lib\Legacy_FIXME\gte($cls, $obj));C(HH\Lib\Legacy_FIXME\eq($cls, $obj));C($cls===$obj);
  print_divider();

  begin_row('lcl', 'v');
    C($vcls<$vlcl);C($vcls<=$vlcl);C($vcls>$vlcl);C($vcls>=$vlcl);C($vcls==$vlcl);C($vcls===$vlcl);
  begin_row('str', 'v');
    C($vcls<$vstr);C($vcls<=$vstr);C($vcls>$vstr);C($vcls>=$vstr);C($vcls==$vstr);C($vcls===$vstr);
  begin_row('cls', 'v');
    C($vcls<$vcls);C($vcls<=$vcls);C($vcls>$vcls);C($vcls>=$vcls);C($vcls==$vcls);C($vcls===$vcls);
  begin_row('obj', 'v');
    C(HH\Lib\Legacy_FIXME\lt($vcls, $vobj));C(HH\Lib\Legacy_FIXME\lte($vcls, $vobj));C(HH\Lib\Legacy_FIXME\gt($vcls, $vobj));C(HH\Lib\Legacy_FIXME\gte($vcls, $vobj));C(HH\Lib\Legacy_FIXME\eq($vcls, $vobj));C($vcls===$vobj);
  print_divider();

  begin_row('lcl', 'w');
    C($wcls<$wlcl);C($wcls<=$wlcl);C($wcls>$wlcl);C($wcls>=$wlcl);C($wcls==$wlcl);C($wcls===$wlcl);
  begin_row('str', 'w');
    C($wcls<$wstr);C($wcls<=$wstr);C($wcls>$wstr);C($wcls>=$wstr);C($wcls==$wstr);C($wcls===$wstr);
  begin_row('cls', 'w');
    C($wcls<$wcls);C($wcls<=$wcls);C($wcls>$wcls);C($wcls>=$wcls);C($wcls==$wcls);C($wcls===$wcls);
  begin_row('obj', 'w');
    C(HH\Lib\Legacy_FIXME\lt($wcls, $wobj));C(HH\Lib\Legacy_FIXME\lte($wcls, $wobj));C(HH\Lib\Legacy_FIXME\gt($wcls, $wobj));C(HH\Lib\Legacy_FIXME\gte($wcls, $wobj));C(HH\Lib\Legacy_FIXME\eq($wcls, $wobj));C($wcls===$wobj);
  print_divider();

  begin_row('lcl', 'p');
    C($pcls<$plcl);C($pcls<=$plcl);C($pcls>$plcl);C($pcls>=$plcl);C($pcls==$plcl);C($pcls===$plcl);
  begin_row('str', 'p');
    C($pcls<$pstr);C($pcls<=$pstr);C($pcls>$pstr);C($pcls>=$pstr);C($pcls==$pstr);C($pcls===$pstr);
  begin_row('cls', 'p');
    C($pcls<$pcls);C($pcls<=$pcls);C($pcls>$pcls);C($pcls>=$pcls);C($pcls==$pcls);C($pcls===$pcls);
  begin_row('obj', 'p');
    C(HH\Lib\Legacy_FIXME\lt($pcls, $pobj));C(HH\Lib\Legacy_FIXME\lte($pcls, $pobj));C(HH\Lib\Legacy_FIXME\gt($pcls, $pobj));C(HH\Lib\Legacy_FIXME\gte($pcls, $pobj));C(HH\Lib\Legacy_FIXME\eq($pcls, $pobj));C($pcls===$pobj);
  print_footer();

  print_header('[static] VAR ? $cls');
  begin_row('lcl');
    C($lcl<$cls);C($lcl<=$cls);C($lcl>$cls);C($lcl>=$cls);C($lcl==$cls);C($lcl===$cls);
  begin_row('str');
    C($str<$cls);C($str<=$cls);C($str>$cls);C($str>=$cls);C($str==$cls);C($str===$cls);
  begin_row('cls');
    C($cls<$cls);C($cls<=$cls);C($cls>$cls);C($cls>=$cls);C($cls==$cls);C($cls===$cls);
  begin_row('obj');
    C(HH\Lib\Legacy_FIXME\lt($obj, $cls));C(HH\Lib\Legacy_FIXME\lte($obj, $cls));C(HH\Lib\Legacy_FIXME\gt($obj, $cls));C(HH\Lib\Legacy_FIXME\gte($obj, $cls));C(HH\Lib\Legacy_FIXME\eq($obj, $cls));C($obj===$cls);
  print_divider();

  begin_row('lcl', 'v');
    C($vlcl<$vcls);C($vlcl<=$vcls);C($vlcl>$vcls);C($vlcl>=$vcls);C($vlcl==$vcls);C($vlcl===$vcls);
  begin_row('str', 'v');
    C($vstr<$vcls);C($vstr<=$vcls);C($vstr>$vcls);C($vstr>=$vcls);C($vstr==$vcls);C($vstr===$vcls);
  begin_row('cls', 'v');
    C($vcls<$vcls);C($vcls<=$vcls);C($vcls>$vcls);C($vcls>=$vcls);C($vcls==$vcls);C($vcls===$vcls);
  begin_row('obj', 'v');
    C(HH\Lib\Legacy_FIXME\lt($vobj, $vcls));C(HH\Lib\Legacy_FIXME\lte($vobj, $vcls));C(HH\Lib\Legacy_FIXME\gt($vobj, $vcls));C(HH\Lib\Legacy_FIXME\gte($vobj, $vcls));C(HH\Lib\Legacy_FIXME\eq($vobj, $vcls));C($vobj===$vcls);
  print_divider();

  begin_row('lcl', 'w');
    C($wlcl<$wcls);C($wlcl<=$wcls);C($wlcl>$wcls);C($wlcl>=$wcls);C($wlcl==$wcls);C($wlcl===$wcls);
  begin_row('str', 'w');
    C($wstr<$wcls);C($wstr<=$wcls);C($wstr>$wcls);C($wstr>=$wcls);C($wstr==$wcls);C($wstr===$wcls);
  begin_row('cls', 'w');
    C($wcls<$wcls);C($wcls<=$wcls);C($wcls>$wcls);C($wcls>=$wcls);C($wcls==$wcls);C($wcls===$wcls);
  begin_row('obj', 'w');
    C(HH\Lib\Legacy_FIXME\lt($wobj, $wcls));C(HH\Lib\Legacy_FIXME\lte($wobj, $wcls));C(HH\Lib\Legacy_FIXME\gt($wobj, $wcls));C(HH\Lib\Legacy_FIXME\gte($wobj, $wcls));C(HH\Lib\Legacy_FIXME\eq($wobj, $wcls));C($wobj===$wcls);
  print_divider();

  begin_row('lcl', 'p');
    C($plcl<$pcls);C($plcl<=$pcls);C($plcl>$pcls);C($plcl>=$pcls);C($plcl==$pcls);C($plcl===$pcls);
  begin_row('str', 'p');
    C($pstr<$pcls);C($pstr<=$pcls);C($pstr>$pcls);C($pstr>=$pcls);C($pstr==$pcls);C($pstr===$pcls);
  begin_row('cls', 'p');
    C($pcls<$pcls);C($pcls<=$pcls);C($pcls>$pcls);C($pcls>=$pcls);C($pcls==$pcls);C($pcls===$pcls);
  begin_row('obj', 'p');
    C(HH\Lib\Legacy_FIXME\lt($pobj, $pcls));C(HH\Lib\Legacy_FIXME\lte($pobj, $pcls));C(HH\Lib\Legacy_FIXME\gt($pobj, $pcls));C(HH\Lib\Legacy_FIXME\gte($pobj, $pcls));C(HH\Lib\Legacy_FIXME\eq($pobj, $pcls));C($pobj===$pcls);
  print_footer();
}

<<__NEVER_INLINE>> function dynamic_compare() :mixed{
  $lcl = LV(foobar::class);
  $str = LV('foobar');
  $obj = LV(new StrObj('foobar'));
  $cls = LV(CLS('foobar'));

  $vlcl = vLV($lcl);
  $vstr = vLV($str);
  $vobj = vLV($obj);
  $vcls = vLV($cls);

  $wlcl = wLV($lcl);
  $wstr = wLV($str);
  $wobj = wLV($obj);
  $wcls = wLV($cls);

  $plcl = pLV($lcl);
  $pstr = pLV($str);
  $pobj = pLV($obj);
  $pcls = pLV($cls);

  //// $lcl

  print_header('[dynamic] $lcl ? VAR');
  begin_row('lcl');
    C($lcl<$lcl);C($lcl<=$lcl);C($lcl>$lcl);C($lcl>=$lcl);C($lcl==$lcl);C($lcl===$lcl);
  begin_row('str');
    C($lcl<$str);C($lcl<=$str);C($lcl>$str);C($lcl>=$str);C($lcl==$str);C($lcl===$str);
  begin_row('cls');
    C($lcl<$cls);C($lcl<=$cls);C($lcl>$cls);C($lcl>=$cls);C($lcl==$cls);C($lcl===$cls);
  begin_row('obj');
    C(HH\Lib\Legacy_FIXME\lt($lcl, $obj));C(HH\Lib\Legacy_FIXME\lte($lcl, $obj));C(HH\Lib\Legacy_FIXME\gt($lcl, $obj));C(HH\Lib\Legacy_FIXME\gte($lcl, $obj));C(HH\Lib\Legacy_FIXME\eq($lcl, $obj));C($lcl===$obj);
  print_divider();

  begin_row('lcl', 'v');
    C($vlcl<$vlcl);C($vlcl<=$vlcl);C($vlcl>$vlcl);C($vlcl>=$vlcl);C($vlcl==$vlcl);C($vlcl===$vlcl);
  begin_row('str', 'v');
    C($vlcl<$vstr);C($vlcl<=$vstr);C($vlcl>$vstr);C($vlcl>=$vstr);C($vlcl==$vstr);C($vlcl===$vstr);
  begin_row('cls', 'v');
    C($vlcl<$vcls);C($vlcl<=$vcls);C($vlcl>$vcls);C($vlcl>=$vcls);C($vlcl==$vcls);C($vlcl===$vcls);
  begin_row('obj', 'v');
    C(HH\Lib\Legacy_FIXME\lt($vlcl, $vobj));C(HH\Lib\Legacy_FIXME\lte($vlcl, $vobj));C(HH\Lib\Legacy_FIXME\gt($vlcl, $vobj));C(HH\Lib\Legacy_FIXME\gte($vlcl, $vobj));C(HH\Lib\Legacy_FIXME\eq($vlcl, $vobj));C($vlcl===$vobj);
  print_divider();

  begin_row('lcl', 'w');
    C($wlcl<$wlcl);C($wlcl<=$wlcl);C($wlcl>$wlcl);C($wlcl>=$wlcl);C($wlcl==$wlcl);C($wlcl===$wlcl);
  begin_row('str', 'w');
    C($wlcl<$wstr);C($wlcl<=$wstr);C($wlcl>$wstr);C($wlcl>=$wstr);C($wlcl==$wstr);C($wlcl===$wstr);
  begin_row('cls', 'w');
    C($wlcl<$wcls);C($wlcl<=$wcls);C($wlcl>$wcls);C($wlcl>=$wcls);C($wlcl==$wcls);C($wlcl===$wcls);
  begin_row('obj', 'w');
    C(HH\Lib\Legacy_FIXME\lt($wlcl, $wobj));C(HH\Lib\Legacy_FIXME\lte($wlcl, $wobj));C(HH\Lib\Legacy_FIXME\gt($wlcl, $wobj));C(HH\Lib\Legacy_FIXME\gte($wlcl, $wobj));C(HH\Lib\Legacy_FIXME\eq($wlcl, $wobj));C($wlcl===$wobj);
  print_divider();

  begin_row('lcl', 'p');
    C($plcl<$plcl);C($plcl<=$plcl);C($plcl>$plcl);C($plcl>=$plcl);C($plcl==$plcl);C($plcl===$plcl);
  begin_row('str', 'p');
    C($plcl<$pstr);C($plcl<=$pstr);C($plcl>$pstr);C($plcl>=$pstr);C($plcl==$pstr);C($plcl===$pstr);
  begin_row('cls', 'p');
    C($plcl<$pcls);C($plcl<=$pcls);C($plcl>$pcls);C($plcl>=$pcls);C($plcl==$pcls);C($plcl===$pcls);
  begin_row('obj', 'p');
    C(HH\Lib\Legacy_FIXME\lt($plcl, $pobj));C(HH\Lib\Legacy_FIXME\lte($plcl, $pobj));C(HH\Lib\Legacy_FIXME\gt($plcl, $pobj));C(HH\Lib\Legacy_FIXME\gte($plcl, $pobj));C(HH\Lib\Legacy_FIXME\eq($plcl, $pobj));C($plcl===$pobj);
  print_footer();

  print_header('[dynamic] VAR ? $lcl');
  begin_row('lcl');
    C($lcl<$lcl);C($lcl<=$lcl);C($lcl>$lcl);C($lcl>=$lcl);C($lcl==$lcl);C($lcl===$lcl);
  begin_row('str');
    C($str<$lcl);C($str<=$lcl);C($str>$lcl);C($str>=$lcl);C($str==$lcl);C($str===$lcl);
  begin_row('cls');
    C($cls<$lcl);C($cls<=$lcl);C($cls>$lcl);C($cls>=$lcl);C($cls==$lcl);C($cls===$lcl);
  begin_row('obj');
    C(HH\Lib\Legacy_FIXME\lt($obj, $lcl));C(HH\Lib\Legacy_FIXME\lte($obj, $lcl));C(HH\Lib\Legacy_FIXME\gt($obj, $lcl));C(HH\Lib\Legacy_FIXME\gte($obj, $lcl));C(HH\Lib\Legacy_FIXME\eq($obj, $lcl));C($obj===$lcl);
  print_divider();

  begin_row('lcl', 'v');
    C($vlcl<$vlcl);C($vlcl<=$vlcl);C($vlcl>$vlcl);C($vlcl>=$vlcl);C($vlcl==$vlcl);C($vlcl===$vlcl);
  begin_row('str', 'v');
    C($vstr<$vlcl);C($vstr<=$vlcl);C($vstr>$vlcl);C($vstr>=$vlcl);C($vstr==$vlcl);C($vstr===$vlcl);
  begin_row('cls', 'v');
    C($vcls<$vlcl);C($vcls<=$vlcl);C($vcls>$vlcl);C($vcls>=$vlcl);C($vcls==$vlcl);C($vcls===$vlcl);
  begin_row('obj', 'v');
    C(HH\Lib\Legacy_FIXME\lt($vobj, $vlcl));C(HH\Lib\Legacy_FIXME\lte($vobj, $vlcl));C(HH\Lib\Legacy_FIXME\gt($vobj, $vlcl));C(HH\Lib\Legacy_FIXME\gte($vobj, $vlcl));C(HH\Lib\Legacy_FIXME\eq($vobj, $vlcl));C($vobj===$vlcl);
  print_divider();

  begin_row('lcl', 'w');
    C($wlcl<$wlcl);C($wlcl<=$wlcl);C($wlcl>$wlcl);C($wlcl>=$wlcl);C($wlcl==$wlcl);C($wlcl===$wlcl);
  begin_row('str', 'w');
    C($wstr<$wlcl);C($wstr<=$wlcl);C($wstr>$wlcl);C($wstr>=$wlcl);C($wstr==$wlcl);C($wstr===$wlcl);
  begin_row('cls', 'w');
    C($wcls<$wlcl);C($wcls<=$wlcl);C($wcls>$wlcl);C($wcls>=$wlcl);C($wcls==$wlcl);C($wcls===$wlcl);
  begin_row('obj', 'w');
    C(HH\Lib\Legacy_FIXME\lt($wobj, $wlcl));C(HH\Lib\Legacy_FIXME\lte($wobj, $wlcl));C(HH\Lib\Legacy_FIXME\gt($wobj, $wlcl));C(HH\Lib\Legacy_FIXME\gte($wobj, $wlcl));C(HH\Lib\Legacy_FIXME\eq($wobj, $wlcl));C($wobj===$wlcl);
  print_divider();

  begin_row('lcl', 'p');
    C($plcl<$plcl);C($plcl<=$plcl);C($plcl>$plcl);C($plcl>=$plcl);C($plcl==$plcl);C($plcl===$plcl);
  begin_row('str', 'p');
    C($pstr<$plcl);C($pstr<=$plcl);C($pstr>$plcl);C($pstr>=$plcl);C($pstr==$plcl);C($pstr===$plcl);
  begin_row('cls', 'p');
    C($pcls<$plcl);C($pcls<=$plcl);C($pcls>$plcl);C($pcls>=$plcl);C($pcls==$plcl);C($pcls===$plcl);
  begin_row('obj', 'p');
    C(HH\Lib\Legacy_FIXME\lt($pobj, $plcl));C(HH\Lib\Legacy_FIXME\lte($pobj, $plcl));C(HH\Lib\Legacy_FIXME\gt($pobj, $plcl));C(HH\Lib\Legacy_FIXME\gte($pobj, $plcl));C(HH\Lib\Legacy_FIXME\eq($pobj, $plcl));C($pobj===$plcl);
  print_footer();

  //// $cls

  print_header('[dynamic] $cls ? VAR');
  begin_row('lcl');
    C($cls<$lcl);C($cls<=$lcl);C($cls>$lcl);C($cls>=$lcl);C($cls==$lcl);C($cls===$lcl);
  begin_row('str');
    C($cls<$str);C($cls<=$str);C($cls>$str);C($cls>=$str);C($cls==$str);C($cls===$str);
  begin_row('cls');
    C($cls<$cls);C($cls<=$cls);C($cls>$cls);C($cls>=$cls);C($cls==$cls);C($cls===$cls);
  begin_row('obj');
    C(HH\Lib\Legacy_FIXME\lt($cls, $obj));C(HH\Lib\Legacy_FIXME\lte($cls, $obj));C(HH\Lib\Legacy_FIXME\gt($cls, $obj));C(HH\Lib\Legacy_FIXME\gte($cls, $obj));C(HH\Lib\Legacy_FIXME\eq($cls, $obj));C($cls===$obj);
  print_divider();

  begin_row('lcl', 'v');
    C($vcls<$vlcl);C($vcls<=$vlcl);C($vcls>$vlcl);C($vcls>=$vlcl);C($vcls==$vlcl);C($vcls===$vlcl);
  begin_row('str', 'v');
    C($vcls<$vstr);C($vcls<=$vstr);C($vcls>$vstr);C($vcls>=$vstr);C($vcls==$vstr);C($vcls===$vstr);
  begin_row('cls', 'v');
    C($vcls<$vcls);C($vcls<=$vcls);C($vcls>$vcls);C($vcls>=$vcls);C($vcls==$vcls);C($vcls===$vcls);
  begin_row('obj', 'v');
    C(HH\Lib\Legacy_FIXME\lt($vcls, $vobj));C(HH\Lib\Legacy_FIXME\lte($vcls, $vobj));C(HH\Lib\Legacy_FIXME\gt($vcls, $vobj));C(HH\Lib\Legacy_FIXME\gte($vcls, $vobj));C(HH\Lib\Legacy_FIXME\eq($vcls, $vobj));C($vcls===$vobj);
  print_divider();

  begin_row('lcl', 'w');
    C($wcls<$wlcl);C($wcls<=$wlcl);C($wcls>$wlcl);C($wcls>=$wlcl);C($wcls==$wlcl);C($wcls===$wlcl);
  begin_row('str', 'w');
    C($wcls<$wstr);C($wcls<=$wstr);C($wcls>$wstr);C($wcls>=$wstr);C($wcls==$wstr);C($wcls===$wstr);
  begin_row('cls', 'w');
    C($wcls<$wcls);C($wcls<=$wcls);C($wcls>$wcls);C($wcls>=$wcls);C($wcls==$wcls);C($wcls===$wcls);
  begin_row('obj', 'w');
    C(HH\Lib\Legacy_FIXME\lt($wcls, $wobj));C(HH\Lib\Legacy_FIXME\lte($wcls, $wobj));C(HH\Lib\Legacy_FIXME\gt($wcls, $wobj));C(HH\Lib\Legacy_FIXME\gte($wcls, $wobj));C(HH\Lib\Legacy_FIXME\eq($wcls, $wobj));C($wcls===$wobj);
  print_divider();

  begin_row('lcl', 'p');
    C($pcls<$plcl);C($pcls<=$plcl);C($pcls>$plcl);C($pcls>=$plcl);C($pcls==$plcl);C($pcls===$plcl);
  begin_row('str', 'p');
    C($pcls<$pstr);C($pcls<=$pstr);C($pcls>$pstr);C($pcls>=$pstr);C($pcls==$pstr);C($pcls===$pstr);
  begin_row('cls', 'p');
    C($pcls<$pcls);C($pcls<=$pcls);C($pcls>$pcls);C($pcls>=$pcls);C($pcls==$pcls);C($pcls===$pcls);
  begin_row('obj', 'p');
    C(HH\Lib\Legacy_FIXME\lt($pcls, $pobj));C(HH\Lib\Legacy_FIXME\lte($pcls, $pobj));C(HH\Lib\Legacy_FIXME\gt($pcls, $pobj));C(HH\Lib\Legacy_FIXME\gte($pcls, $pobj));C(HH\Lib\Legacy_FIXME\eq($pcls, $pobj));C($pcls===$pobj);
  print_footer();

  print_header('[dynamic] VAR ? $cls');
  begin_row('lcl');
    C($lcl<$cls);C($lcl<=$cls);C($lcl>$cls);C($lcl>=$cls);C($lcl==$cls);C($lcl===$cls);
  begin_row('str');
    C($str<$cls);C($str<=$cls);C($str>$cls);C($str>=$cls);C($str==$cls);C($str===$cls);
  begin_row('cls');
    C($cls<$cls);C($cls<=$cls);C($cls>$cls);C($cls>=$cls);C($cls==$cls);C($cls===$cls);
  begin_row('obj');
    C(HH\Lib\Legacy_FIXME\lt($obj, $cls));C(HH\Lib\Legacy_FIXME\lte($obj, $cls));C(HH\Lib\Legacy_FIXME\gt($obj, $cls));C(HH\Lib\Legacy_FIXME\gte($obj, $cls));C(HH\Lib\Legacy_FIXME\eq($obj, $cls));C($obj===$cls);
  print_divider();

  begin_row('lcl', 'v');
    C($vlcl<$vcls);C($vlcl<=$vcls);C($vlcl>$vcls);C($vlcl>=$vcls);C($vlcl==$vcls);C($vlcl===$vcls);
  begin_row('str', 'v');
    C($vstr<$vcls);C($vstr<=$vcls);C($vstr>$vcls);C($vstr>=$vcls);C($vstr==$vcls);C($vstr===$vcls);
  begin_row('cls', 'v');
    C($vcls<$vcls);C($vcls<=$vcls);C($vcls>$vcls);C($vcls>=$vcls);C($vcls==$vcls);C($vcls===$vcls);
  begin_row('obj', 'v');
    C(HH\Lib\Legacy_FIXME\lt($vobj, $vcls));C(HH\Lib\Legacy_FIXME\lte($vobj, $vcls));C(HH\Lib\Legacy_FIXME\gt($vobj, $vcls));C(HH\Lib\Legacy_FIXME\gte($vobj, $vcls));C(HH\Lib\Legacy_FIXME\eq($vobj, $vcls));C($vobj===$vcls);
  print_divider();

  begin_row('lcl', 'w');
    C($wlcl<$wcls);C($wlcl<=$wcls);C($wlcl>$wcls);C($wlcl>=$wcls);C($wlcl==$wcls);C($wlcl===$wcls);
  begin_row('str', 'w');
    C($wstr<$wcls);C($wstr<=$wcls);C($wstr>$wcls);C($wstr>=$wcls);C($wstr==$wcls);C($wstr===$wcls);
  begin_row('cls', 'w');
    C($wcls<$wcls);C($wcls<=$wcls);C($wcls>$wcls);C($wcls>=$wcls);C($wcls==$wcls);C($wcls===$wcls);
  begin_row('obj', 'w');
    C(HH\Lib\Legacy_FIXME\lt($wobj, $wcls));C(HH\Lib\Legacy_FIXME\lte($wobj, $wcls));C(HH\Lib\Legacy_FIXME\gt($wobj, $wcls));C(HH\Lib\Legacy_FIXME\gte($wobj, $wcls));C(HH\Lib\Legacy_FIXME\eq($wobj, $wcls));C($wobj===$wcls);
  print_divider();

  begin_row('lcl', 'p');
    C($plcl<$pcls);C($plcl<=$pcls);C($plcl>$pcls);C($plcl>=$pcls);C($plcl==$pcls);C($plcl===$pcls);
  begin_row('str', 'p');
    C($pstr<$pcls);C($pstr<=$pcls);C($pstr>$pcls);C($pstr>=$pcls);C($pstr==$pcls);C($pstr===$pcls);
  begin_row('cls', 'p');
    C($pcls<$pcls);C($pcls<=$pcls);C($pcls>$pcls);C($pcls>=$pcls);C($pcls==$pcls);C($pcls===$pcls);
  begin_row('obj', 'p');
    C(HH\Lib\Legacy_FIXME\lt($pobj, $pcls));C(HH\Lib\Legacy_FIXME\lte($pobj, $pcls));C(HH\Lib\Legacy_FIXME\gt($pobj, $pcls));C(HH\Lib\Legacy_FIXME\gte($pobj, $pcls));C(HH\Lib\Legacy_FIXME\eq($pobj, $pcls));C($pobj===$pcls);
  print_footer();
}

<<__EntryPoint>>
function main() :mixed{
  static_compare();
  dynamic_compare();
}
