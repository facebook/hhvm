<?hh

class Foo { static function bar() {} }
function LV($x = null): mixed {
  return __hhvm_intrinsics\launder_value($x ?? class_meth(Foo::class, 'bar'));
}

function call_varr(varray $varr): void { var_dump($varr); }
function call_darr(darray $darr): void { var_dump($darr); }
function call_arr (array  $arr ): void { var_dump($arr);  }

function static_ret_varr(): varray { return class_meth(Foo::class, 'bar'); }
function static_ret_darr(): darray { return class_meth(Foo::class, 'bar'); }
function static_ret_arr():  array  { return class_meth(Foo::class, 'bar'); }

function dynamic_ret_varr(): varray { return LV(); }
function dynamic_ret_darr(): darray { return LV(); }
function dynamic_ret_arr():  array  { return LV(); }

function static_call_varr(): void { call_varr(class_meth(Foo::class, 'bar')); }
function static_call_darr(): void { call_darr(class_meth(Foo::class, 'bar')); }
function static_call_arr (): void { call_arr (class_meth(Foo::class, 'bar')); }

function dynamic_call_varr(): void { call_varr(LV()); }
function dynamic_call_darr(): void { call_darr(LV()); }
function dynamic_call_arr (): void { call_arr (LV()); }

////////////////////////////////////////////////////////////////////////////////

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
  printf(" %-4s |", $v ? 'T' : 'F');
}
<<__NEVER_INLINE>> function print_footer() {
  echo "\n+------------+------+------+------+------+------+------+------+\n\n";
}
<<__NEVER_INLINE>> function I(int $v) {
  printf(" %-2d   |", $v);
}

function static_compare_darr(): void {
  $dempty = darray[];
  $dsame = darray[0 => 'Foo', 1 => 'bar'];
  $dother = darray['x' => 'y'];
  $meth = class_meth(Foo::class, 'bar');

  $wdempty = varray[$dempty]; $wdsame = varray[$dsame]; $wdother = varray[$dother];
  $wmeth = varray[$meth];

  print_header('[static] VAR ?? $meth');
  begin_row('dempty');
    C($meth<  $dempty);C($meth<= $dempty);C($meth>  $dempty);C($meth>= $dempty);
    C($meth== $dempty);C($meth===$dempty);I($meth<=>$dempty);
  begin_row('dsame');
    C($meth<  $dsame);C($meth<= $dsame);C($meth>  $dsame);C($meth>= $dsame);
    C($meth== $dsame);C($meth===$dsame);I($meth<=>$dsame);
  begin_row('dother');
    C($meth<  $dother);C($meth<= $dother);C($meth>  $dother);C($meth>= $dother);
    C($meth== $dother);C($meth===$dother);I($meth<=>$dother);

  begin_row('dempty', 'W');
    C($wmeth<  $wdempty);C($wmeth<= $wdempty);C($wmeth>  $wdempty);
    C($wmeth>= $wdempty);C($wmeth== $wdempty);C($wmeth===$wdempty);
    I($wmeth<=>$wdempty);
  begin_row('dsame', 'W');
    C($wmeth<  $wdsame);C($wmeth<= $wdsame);C($wmeth>  $wdsame);
    C($wmeth>= $wdsame);C($wmeth== $wdsame);C($wmeth===$wdsame);
    I($wmeth<=>$wdsame);
  begin_row('dother', 'W');
    C($wmeth<  $wdother);C($wmeth<= $wdother);C($wmeth>  $wdother);
    C($wmeth>= $wdother);C($wmeth== $wdother);C($wmeth===$wdother);
    I($wmeth<=>$wdother);
  print_footer();

  print_header('[static] $meth ?? VAR');
  begin_row('dempty');
    C($dempty<  $meth);C($dempty<= $meth);C($dempty>  $meth);C($dempty>= $meth);
    C($dempty== $meth);C($dempty===$meth);I($dempty<=>$meth);
  begin_row('dsame');
    C($dsame<  $meth);C($dsame<= $meth);C($dsame>  $meth);C($dsame>= $meth);
    C($dsame== $meth);C($dsame===$meth);I($dsame<=>$meth);
  begin_row('dother');
    C($dother<  $meth);C($dother<= $meth);C($dother>  $meth);C($dother>= $meth);
    C($dother== $meth);C($dother===$meth);I($dother<=>$meth);

  begin_row('dempty', 'W');
    C($wdempty<  $wmeth);C($wdempty<= $wmeth);C($wdempty>  $wmeth);
    C($wdempty>= $wmeth);C($wdempty== $wmeth);C($wdempty===$wmeth);
    I($wdempty<=>$wmeth);
  begin_row('dsame', 'W');
    C($wdsame<  $wmeth);C($wdsame<= $wmeth);C($wdsame>  $wmeth);
    C($wdsame>= $wmeth);C($wdsame== $wmeth);C($wdsame===$wmeth);
    I($wdsame<=>$wmeth);
  begin_row('dother', 'W');
    C($wdother<  $wmeth);C($wdother<= $wmeth);C($wdother>  $wmeth);
    C($wdother>= $wmeth);C($wdother== $wmeth);C($wdother===$wmeth);
    I($wdother<=>$wmeth);
  print_footer();
}

function dynamic_compare_darr(): void {
  $dempty = LV(darray[]);
  $dsame = LV(darray[0 => LV('Foo'), 1 => 'bar']);
  $dother = LV(darray['x' => LV('y')]);
  $meth = class_meth(Foo::class, 'bar');

  $wdempty = LV(varray[$dempty]); $wdsame = LV(varray[$dsame]);
  $wdother = LV(varray[$dother]); $wmeth = LV(varray[$meth]);

  print_header('[dynamic] VAR ?? $meth');
  begin_row('dempty');
    C($meth<  $dempty);C($meth<= $dempty);C($meth>  $dempty);C($meth>= $dempty);
    C($meth== $dempty);C($meth===$dempty);I($meth<=>$dempty);
  begin_row('dsame');
    C($meth<  $dsame);C($meth<= $dsame);C($meth>  $dsame);C($meth>= $dsame);
    C($meth== $dsame);C($meth===$dsame);I($meth<=>$dsame);
  begin_row('dother');
    C($meth<  $dother);C($meth<= $dother);C($meth>  $dother);C($meth>= $dother);
    C($meth== $dother);C($meth===$dother);I($meth<=>$dother);

  begin_row('dempty', 'W');
    C($wmeth<  $wdempty);C($wmeth<= $wdempty);C($wmeth>  $wdempty);
    C($wmeth>= $wdempty);C($wmeth== $wdempty);C($wmeth===$wdempty);
    I($wmeth<=>$wdempty);
  begin_row('dsame', 'W');
    C($wmeth<  $wdsame);C($wmeth<= $wdsame);C($wmeth>  $wdsame);
    C($wmeth>= $wdsame);C($wmeth== $wdsame);C($wmeth===$wdsame);
    I($wmeth<=>$wdsame);
  begin_row('dother', 'W');
    C($wmeth<  $wdother);C($wmeth<= $wdother);C($wmeth>  $wdother);
    C($wmeth>= $wdother);C($wmeth== $wdother);C($wmeth===$wdother);
    I($wmeth<=>$wdother);
  print_footer();

  print_header('[dynamic] $meth ?? VAR');
  begin_row('dempty');
    C($dempty<  $meth);C($dempty<= $meth);C($dempty>  $meth);C($dempty>= $meth);
    C($dempty== $meth);C($dempty===$meth);I($dempty<=>$meth);
  begin_row('dsame');
    C($dsame<  $meth);C($dsame<= $meth);C($dsame>  $meth);C($dsame>= $meth);
    C($dsame== $meth);C($dsame===$meth);I($dsame<=>$meth);
  begin_row('dother');
    C($dother<  $meth);C($dother<= $meth);C($dother>  $meth);C($dother>= $meth);
    C($dother== $meth);C($dother===$meth);I($dother<=>$meth);

  begin_row('dempty', 'W');
    C($wdempty<  $wmeth);C($wdempty<= $wmeth);C($wdempty>  $wmeth);
    C($wdempty>= $wmeth);C($wdempty== $wmeth);C($wdempty===$wmeth);
    I($wdempty<=>$wmeth);
  begin_row('dsame', 'W');
    C($wdsame<  $wmeth);C($wdsame<= $wmeth);C($wdsame>  $wmeth);
    C($wdsame>= $wmeth);C($wdsame== $wmeth);C($wdsame===$wmeth);
    I($wdsame<=>$wmeth);
  begin_row('dother', 'W');
    C($wdother<  $wmeth);C($wdother<= $wmeth);C($wdother>  $wmeth);
    C($wdother>= $wmeth);C($wdother== $wmeth);C($wdother===$wmeth);
    I($wdother<=>$wmeth);
  print_footer();
}

function static_compare_varr(): void {
  $vempty = varray[];
  $vsame = varray['Foo', 'bar'];
  $vother = varray['y'];
  $meth = class_meth(Foo::class, 'bar');

  $wvempty = varray[$vempty]; $wvsame = varray[$vsame]; $wvother = varray[$vother];
  $wmeth = varray[$meth];

  print_header('[static] VAR ?? $meth');
  begin_row('vempty');
    C($meth<  $vempty);C($meth<= $vempty);C($meth>  $vempty);C($meth>= $vempty);
    C($meth== $vempty);C($meth===$vempty);I($meth<=>$vempty);
  begin_row('vsame');
    C($meth<  $vsame);C($meth<= $vsame);C($meth>  $vsame);C($meth>= $vsame);
    C($meth== $vsame);C($meth===$vsame);I($meth<=>$vsame);
  begin_row('vother');
    C($meth<  $vother);C($meth<= $vother);C($meth>  $vother);C($meth>= $vother);
    C($meth== $vother);C($meth===$vother);I($meth<=>$vother);

  begin_row('vempty', 'W');
    C($wmeth<  $wvempty);C($wmeth<= $wvempty);C($wmeth>  $wvempty);
    C($wmeth>= $wvempty);C($wmeth== $wvempty);C($wmeth===$wvempty);
    I($wmeth<=>$wvempty);
  begin_row('vsame', 'W');
    C($wmeth<  $wvsame);C($wmeth<= $wvsame);C($wmeth>  $wvsame);
    C($wmeth>= $wvsame);C($wmeth== $wvsame);C($wmeth===$wvsame);
    I($wmeth<=>$wvsame);
  begin_row('vother', 'W');
    C($wmeth<  $wvother);C($wmeth<= $wvother);C($wmeth>  $wvother);
    C($wmeth>= $wvother);C($wmeth== $wvother);C($wmeth===$wvother);
    I($wmeth<=>$wvother);
  print_footer();

  print_header('[static] $meth ?? VAR');
  begin_row('vempty');
    C($vempty<  $meth);C($vempty<= $meth);C($vempty>  $meth);C($vempty>= $meth);
    C($vempty== $meth);C($vempty===$meth);I($vempty<=>$meth);
  begin_row('vsame');
    C($vsame<  $meth);C($vsame<= $meth);C($vsame>  $meth);C($vsame>= $meth);
    C($vsame== $meth);C($vsame===$meth);I($vsame<=>$meth);
  begin_row('vother');
    C($vother<  $meth);C($vother<= $meth);C($vother>  $meth);C($vother>= $meth);
    C($vother== $meth);C($vother===$meth);I($vother<=>$meth);

  begin_row('vempty', 'W');
    C($wvempty<  $wmeth);C($wvempty<= $wmeth);C($wvempty>  $wmeth);
    C($wvempty>= $wmeth);C($wvempty== $wmeth);C($wvempty===$wmeth);
    I($wvempty<=>$wmeth);
  begin_row('vsame', 'W');
    C($wvsame<  $wmeth);C($wvsame<= $wmeth);C($wvsame>  $wmeth);
    C($wvsame>= $wmeth);C($wvsame== $wmeth);C($wvsame===$wmeth);
    I($wvsame<=>$wmeth);
  begin_row('vother', 'W');
    C($wvother<  $wmeth);C($wvother<= $wmeth);C($wvother>  $wmeth);
    C($wvother>= $wmeth);C($wvother== $wmeth);C($wvother===$wmeth);
    I($wvother<=>$wmeth);
  print_footer();
}

function dynamic_compare_varr(): void {
  $vempty = LV(varray[]);
  $vsame = LV(varray[LV('Foo'), 'bar']);
  $vother = LV(varray[LV('y')]);
  $meth = class_meth(Foo::class, 'bar');

  $wvempty = LV(varray[$vempty]); $wvsame = LV(varray[$vsame]);
  $wvother = LV(varray[$vother]); $wmeth = LV(varray[$meth]);

  print_header('[dynamic] VAR ?? $meth');
  begin_row('vempty');
    C($meth<  $vempty);C($meth<= $vempty);C($meth>  $vempty);C($meth>= $vempty);
    C($meth== $vempty);C($meth===$vempty);I($meth<=>$vempty);
  begin_row('vsame');
    C($meth<  $vsame);C($meth<= $vsame);C($meth>  $vsame);C($meth>= $vsame);
    C($meth== $vsame);C($meth===$vsame);I($meth<=>$vsame);
  begin_row('vother');
    C($meth<  $vother);C($meth<= $vother);C($meth>  $vother);C($meth>= $vother);
    C($meth== $vother);C($meth===$vother);I($meth<=>$vother);

  begin_row('vempty', 'W');
    C($wmeth<  $wvempty);C($wmeth<= $wvempty);C($wmeth>  $wvempty);
    C($wmeth>= $wvempty);C($wmeth== $wvempty);C($wmeth===$wvempty);
    I($wmeth<=>$wvempty);
  begin_row('vsame', 'W');
    C($wmeth<  $wvsame);C($wmeth<= $wvsame);C($wmeth>  $wvsame);
    C($wmeth>= $wvsame);C($wmeth== $wvsame);C($wmeth===$wvsame);
    I($wmeth<=>$wvsame);
  begin_row('vother', 'W');
    C($wmeth<  $wvother);C($wmeth<= $wvother);C($wmeth>  $wvother);
    C($wmeth>= $wvother);C($wmeth== $wvother);C($wmeth===$wvother);
    I($wmeth<=>$wvother);
  print_footer();

  print_header('[dynamic] $meth ?? VAR');
  begin_row('vempty');
    C($vempty<  $meth);C($vempty<= $meth);C($vempty>  $meth);C($vempty>= $meth);
    C($vempty== $meth);C($vempty===$meth);I($vempty<=>$meth);
  begin_row('vsame');
    C($vsame<  $meth);C($vsame<= $meth);C($vsame>  $meth);C($vsame>= $meth);
    C($vsame== $meth);C($vsame===$meth);I($vsame<=>$meth);
  begin_row('vother');
    C($vother<  $meth);C($vother<= $meth);C($vother>  $meth);C($vother>= $meth);
    C($vother== $meth);C($vother===$meth);I($vother<=>$meth);

  begin_row('vempty', 'W');
    C($wvempty<  $wmeth);C($wvempty<= $wmeth);C($wvempty>  $wmeth);
    C($wvempty>= $wmeth);C($wvempty== $wmeth);C($wvempty===$wmeth);
    I($wvempty<=>$wmeth);
  begin_row('vsame', 'W');
    C($wvsame<  $wmeth);C($wvsame<= $wmeth);C($wvsame>  $wmeth);
    C($wvsame>= $wmeth);C($wvsame== $wmeth);C($wvsame===$wmeth);
    I($wvsame<=>$wmeth);
  begin_row('vother', 'W');
    C($wvother<  $wmeth);C($wvother<= $wmeth);C($wvother>  $wmeth);
    C($wvother>= $wmeth);C($wvother== $wmeth);C($wvother===$wmeth);
    I($wvother<=>$wmeth);
  print_footer();
}

////////////////////////////////////////////////////////////////////////////////

function is_static() {
  $meth = class_meth(Foo::class, 'bar');
  echo "is_array: ";     echo is_array($meth)        ? "True\n" : "False\n";
  echo "is_dict: ";      echo is_dict($meth)         ? "True\n" : "False\n";
  echo "is_vec: ";       echo is_vec($meth)          ? "True\n" : "False\n";
  echo "is_any_array: "; echo hh\is_any_array($meth) ? "True\n" : "False\n";
}

function is_dynamic() {
  $meth = LV(class_meth(Foo::class, 'bar'));
  echo "is_array: ";     echo is_array($meth)        ? "True\n" : "False\n";
  echo "is_dict: ";      echo is_dict($meth)         ? "True\n" : "False\n";
  echo "is_vec: ";       echo is_vec($meth)          ? "True\n" : "False\n";
  echo "is_any_array: "; echo hh\is_any_array($meth) ? "True\n" : "False\n";
}

////////////////////////////////////////////////////////////////////////////////

<<__EntryPoint>>
function main() {
  static_ret_varr(); static_ret_darr(); static_ret_arr();
  static_ret_varr(); static_ret_darr(); static_ret_arr();

  dynamic_ret_varr(); dynamic_ret_darr(); dynamic_ret_arr();
  dynamic_ret_varr(); dynamic_ret_darr(); dynamic_ret_arr();

  static_call_varr(); static_call_darr(); static_call_arr();
  static_call_varr(); static_call_darr(); static_call_arr();

  dynamic_call_varr(); dynamic_call_darr(); dynamic_call_arr();
  dynamic_call_varr(); dynamic_call_darr(); dynamic_call_arr();

  static_compare_darr(); static_compare_varr();
  static_compare_darr(); static_compare_varr();

  dynamic_compare_darr(); dynamic_compare_varr();
  dynamic_compare_darr(); dynamic_compare_varr();

  is_static(); is_static(); is_dynamic(); is_dynamic();
}
