<?hh

function LV($x) :mixed{ return __hhvm_intrinsics\launder_value($x); }

<<__NEVER_INLINE>> function print_header($title) :mixed{
  echo "$title\n";
  echo "+---------+------+------+------+------+------+------+------+------+------+\n";
  echo "| VAR     | <    | <=   | >    | >=   | ==   | !=   | ===  | !==  | <=>  |\n";
  echo "+=========+======+======+======+======+======+======+======+======+======+";
}
<<__NEVER_INLINE>> function print_footer() :mixed{
  echo "\n+---------+------+------+------+------+------+------+------+------+------+\n\n";
}
<<__NEVER_INLINE>> function begin_row($var) :mixed{
  printf("\n| %-7s |", "\$$var");
}
<<__NEVER_INLINE>> function C(bool $v) :mixed{
  printf(" %-4s |", $v ? 'T' : 'F');
}
<<__NEVER_INLINE>> function I(int $v) :mixed{
  printf(" %-4d |", $v);
}
<<__NEVER_INLINE>> function E() :mixed{
  printf("    * |");
}

<<__NEVER_INLINE>> function compare_varray_empty_static() :mixed{
  $va = vec[];
  $tr = true;
  $fa = false;
  $nu = null;

  print_header('[static] $va ? VAR');
  begin_row('true');
    try { C($va<  $tr); } catch (Exception $_) { E(); }
    try { C($va<= $tr); } catch (Exception $_) { E(); }
    try { C($va > $tr); } catch (Exception $_) { E(); }
    try { C($va >=$tr); } catch (Exception $_) { E(); }
    C($va ==$tr);
    C($va !=$tr);
    C($va===$tr);
    C($va!==$tr);
    try { I($va<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($va<  $fa); } catch (Exception $_) { E(); }
    try { C($va<= $fa); } catch (Exception $_) { E(); }
    try { C($va > $fa); } catch (Exception $_) { E(); }
    try { C($va >=$fa); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($va, $fa));
    C(HH\Lib\Legacy_FIXME\neq($va, $fa));
    C($va===$fa);
    C($va!==$fa);
    try { I($va<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($va<  $nu); } catch (Exception $_) { E(); }
    try { C($va<= $nu); } catch (Exception $_) { E(); }
    try { C($va > $nu); } catch (Exception $_) { E(); }
    try { C($va >=$nu); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($va, $nu));
    C(HH\Lib\Legacy_FIXME\neq($va, $nu));
    C($va===$nu);
    C($va!==$nu);
    try { I($va<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[static] VAR ? $va');
  begin_row('true');
    try { C($tr<  $va); } catch (Exception $_) { E(); }
    try { C($tr<= $va); } catch (Exception $_) { E(); }
    try { C($tr > $va); } catch (Exception $_) { E(); }
    try { C($tr >=$va); } catch (Exception $_) { E(); }
    C($tr ==$va);
    C($tr !=$va);
    C($tr===$va);
    C($tr!==$va);
    try { I($tr<=>$va); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $va); } catch (Exception $_) { E(); }
    try { C($fa<= $va); } catch (Exception $_) { E(); }
    try { C($fa > $va); } catch (Exception $_) { E(); }
    try { C($fa >=$va); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($fa, $va));
    C(HH\Lib\Legacy_FIXME\neq($fa, $va));
    C($fa===$va);
    C($fa!==$va);
    try { I($fa<=>$va); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $va); } catch (Exception $_) { E(); }
    try { C($nu<= $va); } catch (Exception $_) { E(); }
    try { C($nu > $va); } catch (Exception $_) { E(); }
    try { C($nu >=$va); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($nu, $va));
    C(HH\Lib\Legacy_FIXME\neq($nu, $va));
    C($nu===$va);
    C($nu!==$va);
    try { I($nu<=>$va); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_varray_empty_dynamic() :mixed{
  $va = LV(vec[]);
  $tr = LV(true);
  $fa = LV(false);
  $nu = LV(null);

  print_header('[dynamic] $va ? VAR');
  begin_row('true');
    try { C($va<  $tr); } catch (Exception $_) { E(); }
    try { C($va<= $tr); } catch (Exception $_) { E(); }
    try { C($va > $tr); } catch (Exception $_) { E(); }
    try { C($va >=$tr); } catch (Exception $_) { E(); }
    C($va ==$tr);
    C($va !=$tr);
    C($va===$tr);
    C($va!==$tr);
    try { I($va<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($va<  $fa); } catch (Exception $_) { E(); }
    try { C($va<= $fa); } catch (Exception $_) { E(); }
    try { C($va > $fa); } catch (Exception $_) { E(); }
    try { C($va >=$fa); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($va, $fa));
    C(HH\Lib\Legacy_FIXME\neq($va, $fa));
    C($va===$fa);
    C($va!==$fa);
    try { I($va<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($va<  $nu); } catch (Exception $_) { E(); }
    try { C($va<= $nu); } catch (Exception $_) { E(); }
    try { C($va > $nu); } catch (Exception $_) { E(); }
    try { C($va >=$nu); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($va, $nu));
    C(HH\Lib\Legacy_FIXME\neq($va, $nu));
    C($va===$nu);
    C($va!==$nu);
    try { I($va<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[dynamic] VAR ? $va');
  begin_row('true');
    try { C($tr<  $va); } catch (Exception $_) { E(); }
    try { C($tr<= $va); } catch (Exception $_) { E(); }
    try { C($tr > $va); } catch (Exception $_) { E(); }
    try { C($tr >=$va); } catch (Exception $_) { E(); }
    C($tr ==$va);
    C($tr !=$va);
    C($tr===$va);
    C($tr!==$va);
    try { I($tr<=>$va); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $va); } catch (Exception $_) { E(); }
    try { C($fa<= $va); } catch (Exception $_) { E(); }
    try { C($fa > $va); } catch (Exception $_) { E(); }
    try { C($fa >=$va); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($fa, $va));
    C(HH\Lib\Legacy_FIXME\neq($fa, $va));
    C($fa===$va);
    C($fa!==$va);
    try { I($fa<=>$va); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $va); } catch (Exception $_) { E(); }
    try { C($nu<= $va); } catch (Exception $_) { E(); }
    try { C($nu > $va); } catch (Exception $_) { E(); }
    try { C($nu >=$va); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($nu, $va));
    C(HH\Lib\Legacy_FIXME\neq($nu, $va));
    C($nu===$va);
    C($nu!==$va);
    try { I($nu<=>$va); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_varray_nonempty_static() :mixed{
  $vx = vec[42, 'foo'];
  $tr = true;
  $fa = false;
  $nu = null;

  print_header('[static] $vx ? VAR');
  begin_row('true');
    try { C($vx<  $tr); } catch (Exception $_) { E(); }
    try { C($vx<= $tr); } catch (Exception $_) { E(); }
    try { C($vx > $tr); } catch (Exception $_) { E(); }
    try { C($vx >=$tr); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($vx, $tr));
    C(HH\Lib\Legacy_FIXME\neq($vx, $tr));
    C($vx===$tr);
    C($vx!==$tr);
    try { I($vx<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($vx<  $fa); } catch (Exception $_) { E(); }
    try { C($vx<= $fa); } catch (Exception $_) { E(); }
    try { C($vx > $fa); } catch (Exception $_) { E(); }
    try { C($vx >=$fa); } catch (Exception $_) { E(); }
    C($vx ==$fa);
    C($vx !=$fa);
    C($vx===$fa);
    C($vx!==$fa);
    try { I($vx<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($vx<  $nu); } catch (Exception $_) { E(); }
    try { C($vx<= $nu); } catch (Exception $_) { E(); }
    try { C($vx > $nu); } catch (Exception $_) { E(); }
    try { C($vx >=$nu); } catch (Exception $_) { E(); }
    C($vx ==$nu);
    C($vx !=$nu);
    C($vx===$nu);
    C($vx!==$nu);
    try { I($vx<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[static] VAR ? $vx');
  begin_row('true');
    try { C($tr<  $vx); } catch (Exception $_) { E(); }
    try { C($tr<= $vx); } catch (Exception $_) { E(); }
    try { C($tr > $vx); } catch (Exception $_) { E(); }
    try { C($tr >=$vx); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($tr, $vx));
    C(HH\Lib\Legacy_FIXME\neq($tr, $vx));
    C($tr===$vx);
    C($tr!==$vx);
    try { I($tr<=>$vx); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $vx); } catch (Exception $_) { E(); }
    try { C($fa<= $vx); } catch (Exception $_) { E(); }
    try { C($fa > $vx); } catch (Exception $_) { E(); }
    try { C($fa >=$vx); } catch (Exception $_) { E(); }
    C($fa ==$vx);
    C($fa !=$vx);
    C($fa===$vx);
    C($fa!==$vx);
    try { I($fa<=>$vx); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $vx); } catch (Exception $_) { E(); }
    try { C($nu<= $vx); } catch (Exception $_) { E(); }
    try { C($nu > $vx); } catch (Exception $_) { E(); }
    try { C($nu >=$vx); } catch (Exception $_) { E(); }
    C($nu ==$vx);
    C($nu !=$vx);
    C($nu===$vx);
    C($nu!==$vx);
    try { I($nu<=>$vx); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_varray_nonempty_dynamic() :mixed{
  $vx = LV(vec[42, 'foo']);
  $tr = LV(true);
  $fa = LV(false);
  $nu = LV(null);

  print_header('[dynamic] $vx ? VAR');
  begin_row('true');
    try { C($vx<  $tr); } catch (Exception $_) { E(); }
    try { C($vx<= $tr); } catch (Exception $_) { E(); }
    try { C($vx > $tr); } catch (Exception $_) { E(); }
    try { C($vx >=$tr); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($vx, $tr));
    C(HH\Lib\Legacy_FIXME\neq($vx, $tr));
    C($vx===$tr);
    C($vx!==$tr);
    try { I($vx<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($vx<  $fa); } catch (Exception $_) { E(); }
    try { C($vx<= $fa); } catch (Exception $_) { E(); }
    try { C($vx > $fa); } catch (Exception $_) { E(); }
    try { C($vx >=$fa); } catch (Exception $_) { E(); }
    C($vx ==$fa);
    C($vx !=$fa);
    C($vx===$fa);
    C($vx!==$fa);
    try { I($vx<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($vx<  $nu); } catch (Exception $_) { E(); }
    try { C($vx<= $nu); } catch (Exception $_) { E(); }
    try { C($vx > $nu); } catch (Exception $_) { E(); }
    try { C($vx >=$nu); } catch (Exception $_) { E(); }
    C($vx ==$nu);
    C($vx !=$nu);
    C($vx===$nu);
    C($vx!==$nu);
    try { I($vx<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[dynamic] VAR ? $vx');
  begin_row('true');
    try { C($tr<  $vx); } catch (Exception $_) { E(); }
    try { C($tr<= $vx); } catch (Exception $_) { E(); }
    try { C($tr > $vx); } catch (Exception $_) { E(); }
    try { C($tr >=$vx); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($tr, $vx));
    C(HH\Lib\Legacy_FIXME\neq($tr, $vx));
    C($tr===$vx);
    C($tr!==$vx);
    try { I($tr<=>$vx); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $vx); } catch (Exception $_) { E(); }
    try { C($fa<= $vx); } catch (Exception $_) { E(); }
    try { C($fa > $vx); } catch (Exception $_) { E(); }
    try { C($fa >=$vx); } catch (Exception $_) { E(); }
    C($fa ==$vx);
    C($fa !=$vx);
    C($fa===$vx);
    C($fa!==$vx);
    try { I($fa<=>$vx); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $vx); } catch (Exception $_) { E(); }
    try { C($nu<= $vx); } catch (Exception $_) { E(); }
    try { C($nu > $vx); } catch (Exception $_) { E(); }
    try { C($nu >=$vx); } catch (Exception $_) { E(); }
    C($nu ==$vx);
    C($nu !=$vx);
    C($nu===$vx);
    C($nu!==$vx);
    try { I($nu<=>$vx); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_darray_empty_static() :mixed{
  $da = dict[];
  $tr = true;
  $fa = false;
  $nu = null;

  print_header('[static] $da ? VAR');
  begin_row('true');
    try { C($da<  $tr); } catch (Exception $_) { E(); }
    try { C($da<= $tr); } catch (Exception $_) { E(); }
    try { C($da > $tr); } catch (Exception $_) { E(); }
    try { C($da >=$tr); } catch (Exception $_) { E(); }
    C($da ==$tr);
    C($da !=$tr);
    C($da===$tr);
    C($da!==$tr);
    try { I($da<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($da<  $fa); } catch (Exception $_) { E(); }
    try { C($da<= $fa); } catch (Exception $_) { E(); }
    try { C($da > $fa); } catch (Exception $_) { E(); }
    try { C($da >=$fa); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($da, $fa));
    C(HH\Lib\Legacy_FIXME\neq($da, $fa));
    C($da===$fa);
    C($da!==$fa);
    try { I($da<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($da<  $nu); } catch (Exception $_) { E(); }
    try { C($da<= $nu); } catch (Exception $_) { E(); }
    try { C($da > $nu); } catch (Exception $_) { E(); }
    try { C($da >=$nu); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($da, $nu));
    C(HH\Lib\Legacy_FIXME\neq($da, $nu));
    C($da===$nu);
    C($da!==$nu);
    try { I($da<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[static] VAR ? $da');
  begin_row('true');
    try { C($tr<  $da); } catch (Exception $_) { E(); }
    try { C($tr<= $da); } catch (Exception $_) { E(); }
    try { C($tr > $da); } catch (Exception $_) { E(); }
    try { C($tr >=$da); } catch (Exception $_) { E(); }
    C($tr ==$da);
    C($tr !=$da);
    C($tr===$da);
    C($tr!==$da);
    try { I($tr<=>$da); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $da); } catch (Exception $_) { E(); }
    try { C($fa<= $da); } catch (Exception $_) { E(); }
    try { C($fa > $da); } catch (Exception $_) { E(); }
    try { C($fa >=$da); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($fa, $da));
    C(HH\Lib\Legacy_FIXME\neq($fa, $da));
    C($fa===$da);
    C($fa!==$da);
    try { I($fa<=>$da); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $da); } catch (Exception $_) { E(); }
    try { C($nu<= $da); } catch (Exception $_) { E(); }
    try { C($nu > $da); } catch (Exception $_) { E(); }
    try { C($nu >=$da); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($nu, $da));
    C(HH\Lib\Legacy_FIXME\neq($nu, $da));
    C($nu===$da);
    C($nu!==$da);
    try { I($nu<=>$da); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_darray_empty_dynamic() :mixed{
  $da = LV(dict[]);
  $tr = LV(true);
  $fa = LV(false);
  $nu = LV(null);

  print_header('[dynamic] $da ? VAR');
  begin_row('true');
    try { C($da<  $tr); } catch (Exception $_) { E(); }
    try { C($da<= $tr); } catch (Exception $_) { E(); }
    try { C($da > $tr); } catch (Exception $_) { E(); }
    try { C($da >=$tr); } catch (Exception $_) { E(); }
    C($da ==$tr);
    C($da !=$tr);
    C($da===$tr);
    C($da!==$tr);
    try { I($da<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($da<  $fa); } catch (Exception $_) { E(); }
    try { C($da<= $fa); } catch (Exception $_) { E(); }
    try { C($da > $fa); } catch (Exception $_) { E(); }
    try { C($da >=$fa); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($da, $fa));
    C(HH\Lib\Legacy_FIXME\neq($da, $fa));
    C($da===$fa);
    C($da!==$fa);
    try { I($da<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($da<  $nu); } catch (Exception $_) { E(); }
    try { C($da<= $nu); } catch (Exception $_) { E(); }
    try { C($da > $nu); } catch (Exception $_) { E(); }
    try { C($da >=$nu); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($da, $nu));
    C(HH\Lib\Legacy_FIXME\neq($da, $nu));
    C($da===$nu);
    C($da!==$nu);
    try { I($da<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[dynamic] VAR ? $da');
  begin_row('true');
    try { C($tr<  $da); } catch (Exception $_) { E(); }
    try { C($tr<= $da); } catch (Exception $_) { E(); }
    try { C($tr > $da); } catch (Exception $_) { E(); }
    try { C($tr >=$da); } catch (Exception $_) { E(); }
    C($tr ==$da);
    C($tr !=$da);
    C($tr===$da);
    C($tr!==$da);
    try { I($tr<=>$da); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $da); } catch (Exception $_) { E(); }
    try { C($fa<= $da); } catch (Exception $_) { E(); }
    try { C($fa > $da); } catch (Exception $_) { E(); }
    try { C($fa >=$da); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($fa, $da));
    C(HH\Lib\Legacy_FIXME\neq($fa, $da));
    C($fa===$da);
    C($fa!==$da);
    try { I($fa<=>$da); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $da); } catch (Exception $_) { E(); }
    try { C($nu<= $da); } catch (Exception $_) { E(); }
    try { C($nu > $da); } catch (Exception $_) { E(); }
    try { C($nu >=$da); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($nu, $da));
    C(HH\Lib\Legacy_FIXME\neq($nu, $da));
    C($nu===$da);
    C($nu!==$da);
    try { I($nu<=>$da); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_darray_nonempty_static() :mixed{
  $dx = dict['foo' => 42, 'bar' => 'baz'];
  $tr = true;
  $fa = false;
  $nu = null;

  print_header('[static] $dx ? VAR');
  begin_row('true');
    try { C($dx<  $tr); } catch (Exception $_) { E(); }
    try { C($dx<= $tr); } catch (Exception $_) { E(); }
    try { C($dx > $tr); } catch (Exception $_) { E(); }
    try { C($dx >=$tr); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($dx, $tr));
    C(HH\Lib\Legacy_FIXME\neq($dx, $tr));
    C($dx===$tr);
    C($dx!==$tr);
    try { I($dx<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($dx<  $fa); } catch (Exception $_) { E(); }
    try { C($dx<= $fa); } catch (Exception $_) { E(); }
    try { C($dx > $fa); } catch (Exception $_) { E(); }
    try { C($dx >=$fa); } catch (Exception $_) { E(); }
    C($dx ==$fa);
    C($dx !=$fa);
    C($dx===$fa);
    C($dx!==$fa);
    try { I($dx<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($dx<  $nu); } catch (Exception $_) { E(); }
    try { C($dx<= $nu); } catch (Exception $_) { E(); }
    try { C($dx > $nu); } catch (Exception $_) { E(); }
    try { C($dx >=$nu); } catch (Exception $_) { E(); }
    C($dx ==$nu);
    C($dx !=$nu);
    C($dx===$nu);
    C($dx!==$nu);
    try { I($dx<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[static] VAR ? $dx');
  begin_row('true');
    try { C($tr<  $dx); } catch (Exception $_) { E(); }
    try { C($tr<= $dx); } catch (Exception $_) { E(); }
    try { C($tr > $dx); } catch (Exception $_) { E(); }
    try { C($tr >=$dx); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($tr, $dx));
    C(HH\Lib\Legacy_FIXME\neq($tr, $dx));
    C($tr===$dx);
    C($tr!==$dx);
    try { I($tr<=>$dx); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $dx); } catch (Exception $_) { E(); }
    try { C($fa<= $dx); } catch (Exception $_) { E(); }
    try { C($fa > $dx); } catch (Exception $_) { E(); }
    try { C($fa >=$dx); } catch (Exception $_) { E(); }
    C($fa ==$dx);
    C($fa !=$dx);
    C($fa===$dx);
    C($fa!==$dx);
    try { I($fa<=>$dx); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $dx); } catch (Exception $_) { E(); }
    try { C($nu<= $dx); } catch (Exception $_) { E(); }
    try { C($nu > $dx); } catch (Exception $_) { E(); }
    try { C($nu >=$dx); } catch (Exception $_) { E(); }
    C($nu ==$dx);
    C($nu !=$dx);
    C($nu===$dx);
    C($nu!==$dx);
    try { I($nu<=>$dx); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_darray_nonempty_dynamic() :mixed{
  $dx = LV(dict['foo' => 42, 'bar' => 'baz']);
  $tr = LV(true);
  $fa = LV(false);
  $nu = LV(null);

  print_header('[dynamic] $dx ? VAR');
  begin_row('true');
    try { C($dx<  $tr); } catch (Exception $_) { E(); }
    try { C($dx<= $tr); } catch (Exception $_) { E(); }
    try { C($dx > $tr); } catch (Exception $_) { E(); }
    try { C($dx >=$tr); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($dx, $tr));
    C(HH\Lib\Legacy_FIXME\neq($dx, $tr));
    C($dx===$tr);
    C($dx!==$tr);
    try { I($dx<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($dx<  $fa); } catch (Exception $_) { E(); }
    try { C($dx<= $fa); } catch (Exception $_) { E(); }
    try { C($dx > $fa); } catch (Exception $_) { E(); }
    try { C($dx >=$fa); } catch (Exception $_) { E(); }
    C($dx ==$fa);
    C($dx !=$fa);
    C($dx===$fa);
    C($dx!==$fa);
    try { I($dx<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($dx<  $nu); } catch (Exception $_) { E(); }
    try { C($dx<= $nu); } catch (Exception $_) { E(); }
    try { C($dx > $nu); } catch (Exception $_) { E(); }
    try { C($dx >=$nu); } catch (Exception $_) { E(); }
    C($dx ==$nu);
    C($dx !=$nu);
    C($dx===$nu);
    C($dx!==$nu);
    try { I($dx<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[dynamic] VAR ? $dx');
  begin_row('true');
    try { C($tr<  $dx); } catch (Exception $_) { E(); }
    try { C($tr<= $dx); } catch (Exception $_) { E(); }
    try { C($tr > $dx); } catch (Exception $_) { E(); }
    try { C($tr >=$dx); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($tr, $dx));
    C(HH\Lib\Legacy_FIXME\neq($tr, $dx));
    C($tr===$dx);
    C($tr!==$dx);
    try { I($tr<=>$dx); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $dx); } catch (Exception $_) { E(); }
    try { C($fa<= $dx); } catch (Exception $_) { E(); }
    try { C($fa > $dx); } catch (Exception $_) { E(); }
    try { C($fa >=$dx); } catch (Exception $_) { E(); }
    C($fa ==$dx);
    C($fa !=$dx);
    C($fa===$dx);
    C($fa!==$dx);
    try { I($fa<=>$dx); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $dx); } catch (Exception $_) { E(); }
    try { C($nu<= $dx); } catch (Exception $_) { E(); }
    try { C($nu > $dx); } catch (Exception $_) { E(); }
    try { C($nu >=$dx); } catch (Exception $_) { E(); }
    C($nu ==$dx);
    C($nu !=$dx);
    C($nu===$dx);
    C($nu!==$dx);
    try { I($nu<=>$dx); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_vec_empty_static() :mixed{
  $ve = vec[];
  $tr = true;
  $fa = false;
  $nu = null;

  print_header('[static] $ve ? VAR');
  begin_row('true');
    try { C($ve<  $tr); } catch (Exception $_) { E(); }
    try { C($ve<= $tr); } catch (Exception $_) { E(); }
    try { C($ve > $tr); } catch (Exception $_) { E(); }
    try { C($ve >=$tr); } catch (Exception $_) { E(); }
    C($ve ==$tr);
    C($ve !=$tr);
    C($ve===$tr);
    C($ve!==$tr);
    try { I($ve<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($ve<  $fa); } catch (Exception $_) { E(); }
    try { C($ve<= $fa); } catch (Exception $_) { E(); }
    try { C($ve > $fa); } catch (Exception $_) { E(); }
    try { C($ve >=$fa); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($ve, $fa));
    C(HH\Lib\Legacy_FIXME\neq($ve, $fa));
    C($ve===$fa);
    C($ve!==$fa);
    try { I($ve<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($ve<  $nu); } catch (Exception $_) { E(); }
    try { C($ve<= $nu); } catch (Exception $_) { E(); }
    try { C($ve > $nu); } catch (Exception $_) { E(); }
    try { C($ve >=$nu); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($ve, $nu));
    C(HH\Lib\Legacy_FIXME\neq($ve, $nu));
    C($ve===$nu);
    C($ve!==$nu);
    try { I($ve<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[static] VAR ? $ve');
  begin_row('true');
    try { C($tr<  $ve); } catch (Exception $_) { E(); }
    try { C($tr<= $ve); } catch (Exception $_) { E(); }
    try { C($tr > $ve); } catch (Exception $_) { E(); }
    try { C($tr >=$ve); } catch (Exception $_) { E(); }
    C($tr ==$ve);
    C($tr !=$ve);
    C($tr===$ve);
    C($tr!==$ve);
    try { I($tr<=>$ve); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $ve); } catch (Exception $_) { E(); }
    try { C($fa<= $ve); } catch (Exception $_) { E(); }
    try { C($fa > $ve); } catch (Exception $_) { E(); }
    try { C($fa >=$ve); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($fa, $ve));
    C(HH\Lib\Legacy_FIXME\neq($fa, $ve));
    C($fa===$ve);
    C($fa!==$ve);
    try { I($fa<=>$ve); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $ve); } catch (Exception $_) { E(); }
    try { C($nu<= $ve); } catch (Exception $_) { E(); }
    try { C($nu > $ve); } catch (Exception $_) { E(); }
    try { C($nu >=$ve); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($nu, $ve));
    C(HH\Lib\Legacy_FIXME\neq($nu, $ve));
    C($nu===$ve);
    C($nu!==$ve);
    try { I($nu<=>$ve); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_vec_empty_dynamic() :mixed{
  $ve = LV(vec[]);
  $tr = LV(true);
  $fa = LV(false);
  $nu = LV(null);

  print_header('[dynamic] $ve ? VAR');
  begin_row('true');
    try { C($ve<  $tr); } catch (Exception $_) { E(); }
    try { C($ve<= $tr); } catch (Exception $_) { E(); }
    try { C($ve > $tr); } catch (Exception $_) { E(); }
    try { C($ve >=$tr); } catch (Exception $_) { E(); }
    C($ve ==$tr);
    C($ve !=$tr);
    C($ve===$tr);
    C($ve!==$tr);
    try { I($ve<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($ve<  $fa); } catch (Exception $_) { E(); }
    try { C($ve<= $fa); } catch (Exception $_) { E(); }
    try { C($ve > $fa); } catch (Exception $_) { E(); }
    try { C($ve >=$fa); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($ve, $fa));
    C(HH\Lib\Legacy_FIXME\neq($ve, $fa));
    C($ve===$fa);
    C($ve!==$fa);
    try { I($ve<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($ve<  $nu); } catch (Exception $_) { E(); }
    try { C($ve<= $nu); } catch (Exception $_) { E(); }
    try { C($ve > $nu); } catch (Exception $_) { E(); }
    try { C($ve >=$nu); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($ve, $nu));
    C(HH\Lib\Legacy_FIXME\neq($ve, $nu));
    C($ve===$nu);
    C($ve!==$nu);
    try { I($ve<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[dynamic] VAR ? $ve');
  begin_row('true');
    try { C($tr<  $ve); } catch (Exception $_) { E(); }
    try { C($tr<= $ve); } catch (Exception $_) { E(); }
    try { C($tr > $ve); } catch (Exception $_) { E(); }
    try { C($tr >=$ve); } catch (Exception $_) { E(); }
    C($tr ==$ve);
    C($tr !=$ve);
    C($tr===$ve);
    C($tr!==$ve);
    try { I($tr<=>$ve); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $ve); } catch (Exception $_) { E(); }
    try { C($fa<= $ve); } catch (Exception $_) { E(); }
    try { C($fa > $ve); } catch (Exception $_) { E(); }
    try { C($fa >=$ve); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($fa, $ve));
    C(HH\Lib\Legacy_FIXME\neq($fa, $ve));
    C($fa===$ve);
    C($fa!==$ve);
    try { I($fa<=>$ve); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $ve); } catch (Exception $_) { E(); }
    try { C($nu<= $ve); } catch (Exception $_) { E(); }
    try { C($nu > $ve); } catch (Exception $_) { E(); }
    try { C($nu >=$ve); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($nu, $ve));
    C(HH\Lib\Legacy_FIXME\neq($nu, $ve));
    C($nu===$ve);
    C($nu!==$ve);
    try { I($nu<=>$ve); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_vec_nonempty_static() :mixed{
  $vz = vec[42, 'foo'];
  $tr = true;
  $fa = false;
  $nu = null;

  print_header('[static] $vz ? VAR');
  begin_row('true');
    try { C($vz<  $tr); } catch (Exception $_) { E(); }
    try { C($vz<= $tr); } catch (Exception $_) { E(); }
    try { C($vz > $tr); } catch (Exception $_) { E(); }
    try { C($vz >=$tr); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($vz, $tr));
    C(HH\Lib\Legacy_FIXME\neq($vz, $tr));
    C($vz===$tr);
    C($vz!==$tr);
    try { I($vz<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($vz<  $fa); } catch (Exception $_) { E(); }
    try { C($vz<= $fa); } catch (Exception $_) { E(); }
    try { C($vz > $fa); } catch (Exception $_) { E(); }
    try { C($vz >=$fa); } catch (Exception $_) { E(); }
    C($vz ==$fa);
    C($vz !=$fa);
    C($vz===$fa);
    C($vz!==$fa);
    try { I($vz<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($vz<  $nu); } catch (Exception $_) { E(); }
    try { C($vz<= $nu); } catch (Exception $_) { E(); }
    try { C($vz > $nu); } catch (Exception $_) { E(); }
    try { C($vz >=$nu); } catch (Exception $_) { E(); }
    C($vz ==$nu);
    C($vz !=$nu);
    C($vz===$nu);
    C($vz!==$nu);
    try { I($vz<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[static] VAR ? $vz');
  begin_row('true');
    try { C($tr<  $vz); } catch (Exception $_) { E(); }
    try { C($tr<= $vz); } catch (Exception $_) { E(); }
    try { C($tr > $vz); } catch (Exception $_) { E(); }
    try { C($tr >=$vz); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($tr, $vz));
    C(HH\Lib\Legacy_FIXME\neq($tr, $vz));
    C($tr===$vz);
    C($tr!==$vz);
    try { I($tr<=>$vz); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $vz); } catch (Exception $_) { E(); }
    try { C($fa<= $vz); } catch (Exception $_) { E(); }
    try { C($fa > $vz); } catch (Exception $_) { E(); }
    try { C($fa >=$vz); } catch (Exception $_) { E(); }
    C($fa ==$vz);
    C($fa !=$vz);
    C($fa===$vz);
    C($fa!==$vz);
    try { I($fa<=>$vz); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $vz); } catch (Exception $_) { E(); }
    try { C($nu<= $vz); } catch (Exception $_) { E(); }
    try { C($nu > $vz); } catch (Exception $_) { E(); }
    try { C($nu >=$vz); } catch (Exception $_) { E(); }
    C($nu ==$vz);
    C($nu !=$vz);
    C($nu===$vz);
    C($nu!==$vz);
    try { I($nu<=>$vz); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_vec_nonempty_dynamic() :mixed{
  $vz = LV(vec[42, 'foo']);
  $tr = LV(true);
  $fa = LV(false);
  $nu = LV(null);

  print_header('[dynamic] $vz ? VAR');
  begin_row('true');
    try { C($vz<  $tr); } catch (Exception $_) { E(); }
    try { C($vz<= $tr); } catch (Exception $_) { E(); }
    try { C($vz > $tr); } catch (Exception $_) { E(); }
    try { C($vz >=$tr); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($vz, $tr));
    C(HH\Lib\Legacy_FIXME\neq($vz, $tr));
    C($vz===$tr);
    C($vz!==$tr);
    try { I($vz<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($vz<  $fa); } catch (Exception $_) { E(); }
    try { C($vz<= $fa); } catch (Exception $_) { E(); }
    try { C($vz > $fa); } catch (Exception $_) { E(); }
    try { C($vz >=$fa); } catch (Exception $_) { E(); }
    C($vz ==$fa);
    C($vz !=$fa);
    C($vz===$fa);
    C($vz!==$fa);
    try { I($vz<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($vz<  $nu); } catch (Exception $_) { E(); }
    try { C($vz<= $nu); } catch (Exception $_) { E(); }
    try { C($vz > $nu); } catch (Exception $_) { E(); }
    try { C($vz >=$nu); } catch (Exception $_) { E(); }
    C($vz ==$nu);
    C($vz !=$nu);
    C($vz===$nu);
    C($vz!==$nu);
    try { I($vz<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[dynamic] VAR ? $vz');
  begin_row('true');
    try { C($tr<  $vz); } catch (Exception $_) { E(); }
    try { C($tr<= $vz); } catch (Exception $_) { E(); }
    try { C($tr > $vz); } catch (Exception $_) { E(); }
    try { C($tr >=$vz); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($tr, $vz));
    C(HH\Lib\Legacy_FIXME\neq($tr, $vz));
    C($tr===$vz);
    C($tr!==$vz);
    try { I($tr<=>$vz); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $vz); } catch (Exception $_) { E(); }
    try { C($fa<= $vz); } catch (Exception $_) { E(); }
    try { C($fa > $vz); } catch (Exception $_) { E(); }
    try { C($fa >=$vz); } catch (Exception $_) { E(); }
    C($fa ==$vz);
    C($fa !=$vz);
    C($fa===$vz);
    C($fa!==$vz);
    try { I($fa<=>$vz); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $vz); } catch (Exception $_) { E(); }
    try { C($nu<= $vz); } catch (Exception $_) { E(); }
    try { C($nu > $vz); } catch (Exception $_) { E(); }
    try { C($nu >=$vz); } catch (Exception $_) { E(); }
    C($nu ==$vz);
    C($nu !=$vz);
    C($nu===$vz);
    C($nu!==$vz);
    try { I($nu<=>$vz); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_dict_empty_static() :mixed{
  $di = dict[];
  $tr = true;
  $fa = false;
  $nu = null;

  print_header('[static] $di ? VAR');
  begin_row('true');
    try { C($di<  $tr); } catch (Exception $_) { E(); }
    try { C($di<= $tr); } catch (Exception $_) { E(); }
    try { C($di > $tr); } catch (Exception $_) { E(); }
    try { C($di >=$tr); } catch (Exception $_) { E(); }
    C($di ==$tr);
    C($di !=$tr);
    C($di===$tr);
    C($di!==$tr);
    try { I($di<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($di<  $fa); } catch (Exception $_) { E(); }
    try { C($di<= $fa); } catch (Exception $_) { E(); }
    try { C($di > $fa); } catch (Exception $_) { E(); }
    try { C($di >=$fa); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($di, $fa));
    C(HH\Lib\Legacy_FIXME\neq($di, $fa));
    C($di===$fa);
    C($di!==$fa);
    try { I($di<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($di<  $nu); } catch (Exception $_) { E(); }
    try { C($di<= $nu); } catch (Exception $_) { E(); }
    try { C($di > $nu); } catch (Exception $_) { E(); }
    try { C($di >=$nu); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($di, $nu));
    C(HH\Lib\Legacy_FIXME\neq($di, $nu));
    C($di===$nu);
    C($di!==$nu);
    try { I($di<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[static] VAR ? $di');
  begin_row('true');
    try { C($tr<  $di); } catch (Exception $_) { E(); }
    try { C($tr<= $di); } catch (Exception $_) { E(); }
    try { C($tr > $di); } catch (Exception $_) { E(); }
    try { C($tr >=$di); } catch (Exception $_) { E(); }
    C($tr ==$di);
    C($tr !=$di);
    C($tr===$di);
    C($tr!==$di);
    try { I($tr<=>$di); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $di); } catch (Exception $_) { E(); }
    try { C($fa<= $di); } catch (Exception $_) { E(); }
    try { C($fa > $di); } catch (Exception $_) { E(); }
    try { C($fa >=$di); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($fa, $di));
    C(HH\Lib\Legacy_FIXME\neq($fa, $di));
    C($fa===$di);
    C($fa!==$di);
    try { I($fa<=>$di); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $di); } catch (Exception $_) { E(); }
    try { C($nu<= $di); } catch (Exception $_) { E(); }
    try { C($nu > $di); } catch (Exception $_) { E(); }
    try { C($nu >=$di); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($nu, $di));
    C(HH\Lib\Legacy_FIXME\neq($nu, $di));
    C($nu===$di);
    C($nu!==$di);
    try { I($nu<=>$di); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_dict_empty_dynamic() :mixed{
  $di = LV(dict[]);
  $tr = LV(true);
  $fa = LV(false);
  $nu = LV(null);

  print_header('[dynamic] $di ? VAR');
  begin_row('true');
    try { C($di<  $tr); } catch (Exception $_) { E(); }
    try { C($di<= $tr); } catch (Exception $_) { E(); }
    try { C($di > $tr); } catch (Exception $_) { E(); }
    try { C($di >=$tr); } catch (Exception $_) { E(); }
    C($di ==$tr);
    C($di !=$tr);
    C($di===$tr);
    C($di!==$tr);
    try { I($di<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($di<  $fa); } catch (Exception $_) { E(); }
    try { C($di<= $fa); } catch (Exception $_) { E(); }
    try { C($di > $fa); } catch (Exception $_) { E(); }
    try { C($di >=$fa); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($di, $fa));
    C(HH\Lib\Legacy_FIXME\neq($di, $fa));
    C($di===$fa);
    C($di!==$fa);
    try { I($di<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($di<  $nu); } catch (Exception $_) { E(); }
    try { C($di<= $nu); } catch (Exception $_) { E(); }
    try { C($di > $nu); } catch (Exception $_) { E(); }
    try { C($di >=$nu); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($di, $nu));
    C(HH\Lib\Legacy_FIXME\neq($di, $nu));
    C($di===$nu);
    C($di!==$nu);
    try { I($di<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[dynamic] VAR ? $di');
  begin_row('true');
    try { C($tr<  $di); } catch (Exception $_) { E(); }
    try { C($tr<= $di); } catch (Exception $_) { E(); }
    try { C($tr > $di); } catch (Exception $_) { E(); }
    try { C($tr >=$di); } catch (Exception $_) { E(); }
    C($tr ==$di);
    C($tr !=$di);
    C($tr===$di);
    C($tr!==$di);
    try { I($tr<=>$di); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $di); } catch (Exception $_) { E(); }
    try { C($fa<= $di); } catch (Exception $_) { E(); }
    try { C($fa > $di); } catch (Exception $_) { E(); }
    try { C($fa >=$di); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($fa, $di));
    C(HH\Lib\Legacy_FIXME\neq($fa, $di));
    C($fa===$di);
    C($fa!==$di);
    try { I($fa<=>$di); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $di); } catch (Exception $_) { E(); }
    try { C($nu<= $di); } catch (Exception $_) { E(); }
    try { C($nu > $di); } catch (Exception $_) { E(); }
    try { C($nu >=$di); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($nu, $di));
    C(HH\Lib\Legacy_FIXME\neq($nu, $di));
    C($nu===$di);
    C($nu!==$di);
    try { I($nu<=>$di); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_dict_nonempty_static() :mixed{
  $dz = dict['foo' => 42, 'bar' => 'baz'];
  $tr = true;
  $fa = false;
  $nu = null;

  print_header('[static] $dz ? VAR');
  begin_row('true');
    try { C($dz<  $tr); } catch (Exception $_) { E(); }
    try { C($dz<= $tr); } catch (Exception $_) { E(); }
    try { C($dz > $tr); } catch (Exception $_) { E(); }
    try { C($dz >=$tr); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($dz, $tr));
    C(HH\Lib\Legacy_FIXME\neq($dz, $tr));
    C($dz===$tr);
    C($dz!==$tr);
    try { I($dz<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($dz<  $fa); } catch (Exception $_) { E(); }
    try { C($dz<= $fa); } catch (Exception $_) { E(); }
    try { C($dz > $fa); } catch (Exception $_) { E(); }
    try { C($dz >=$fa); } catch (Exception $_) { E(); }
    C($dz ==$fa);
    C($dz !=$fa);
    C($dz===$fa);
    C($dz!==$fa);
    try { I($dz<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($dz<  $nu); } catch (Exception $_) { E(); }
    try { C($dz<= $nu); } catch (Exception $_) { E(); }
    try { C($dz > $nu); } catch (Exception $_) { E(); }
    try { C($dz >=$nu); } catch (Exception $_) { E(); }
    C($dz ==$nu);
    C($dz !=$nu);
    C($dz===$nu);
    C($dz!==$nu);
    try { I($dz<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[static] VAR ? $dz');
  begin_row('true');
    try { C($tr<  $dz); } catch (Exception $_) { E(); }
    try { C($tr<= $dz); } catch (Exception $_) { E(); }
    try { C($tr > $dz); } catch (Exception $_) { E(); }
    try { C($tr >=$dz); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($tr, $dz));
    C(HH\Lib\Legacy_FIXME\neq($tr, $dz));
    C($tr===$dz);
    C($tr!==$dz);
    try { I($tr<=>$dz); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $dz); } catch (Exception $_) { E(); }
    try { C($fa<= $dz); } catch (Exception $_) { E(); }
    try { C($fa > $dz); } catch (Exception $_) { E(); }
    try { C($fa >=$dz); } catch (Exception $_) { E(); }
    C($fa ==$dz);
    C($fa !=$dz);
    C($fa===$dz);
    C($fa!==$dz);
    try { I($fa<=>$dz); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $dz); } catch (Exception $_) { E(); }
    try { C($nu<= $dz); } catch (Exception $_) { E(); }
    try { C($nu > $dz); } catch (Exception $_) { E(); }
    try { C($nu >=$dz); } catch (Exception $_) { E(); }
    C($nu ==$dz);
    C($nu !=$dz);
    C($nu===$dz);
    C($nu!==$dz);
    try { I($nu<=>$dz); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_dict_nonempty_dynamic() :mixed{
  $dz = LV(dict['foo' => 42, 'bar' => 'baz']);
  $tr = LV(true);
  $fa = LV(false);
  $nu = LV(null);

  print_header('[dynamic] $dz ? VAR');
  begin_row('true');
    try { C($dz<  $tr); } catch (Exception $_) { E(); }
    try { C($dz<= $tr); } catch (Exception $_) { E(); }
    try { C($dz > $tr); } catch (Exception $_) { E(); }
    try { C($dz >=$tr); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($dz, $tr));
    C(HH\Lib\Legacy_FIXME\neq($dz, $tr));
    C($dz===$tr);
    C($dz!==$tr);
    try { I($dz<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($dz<  $fa); } catch (Exception $_) { E(); }
    try { C($dz<= $fa); } catch (Exception $_) { E(); }
    try { C($dz > $fa); } catch (Exception $_) { E(); }
    try { C($dz >=$fa); } catch (Exception $_) { E(); }
    C($dz ==$fa);
    C($dz !=$fa);
    C($dz===$fa);
    C($dz!==$fa);
    try { I($dz<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($dz<  $nu); } catch (Exception $_) { E(); }
    try { C($dz<= $nu); } catch (Exception $_) { E(); }
    try { C($dz > $nu); } catch (Exception $_) { E(); }
    try { C($dz >=$nu); } catch (Exception $_) { E(); }
    C($dz ==$nu);
    C($dz !=$nu);
    C($dz===$nu);
    C($dz!==$nu);
    try { I($dz<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[dynamic] VAR ? $dz');
  begin_row('true');
    try { C($tr<  $dz); } catch (Exception $_) { E(); }
    try { C($tr<= $dz); } catch (Exception $_) { E(); }
    try { C($tr > $dz); } catch (Exception $_) { E(); }
    try { C($tr >=$dz); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($tr, $dz));
    C(HH\Lib\Legacy_FIXME\neq($tr, $dz));
    C($tr===$dz);
    C($tr!==$dz);
    try { I($tr<=>$dz); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $dz); } catch (Exception $_) { E(); }
    try { C($fa<= $dz); } catch (Exception $_) { E(); }
    try { C($fa > $dz); } catch (Exception $_) { E(); }
    try { C($fa >=$dz); } catch (Exception $_) { E(); }
    C($fa ==$dz);
    C($fa !=$dz);
    C($fa===$dz);
    C($fa!==$dz);
    try { I($fa<=>$dz); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $dz); } catch (Exception $_) { E(); }
    try { C($nu<= $dz); } catch (Exception $_) { E(); }
    try { C($nu > $dz); } catch (Exception $_) { E(); }
    try { C($nu >=$dz); } catch (Exception $_) { E(); }
    C($nu ==$dz);
    C($nu !=$dz);
    C($nu===$dz);
    C($nu!==$dz);
    try { I($nu<=>$dz); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_keyset_empty_static() :mixed{
  $ks = keyset[];
  $tr = true;
  $fa = false;
  $nu = null;

  print_header('[static] $ks ? VAR');
  begin_row('true');
    try { C($ks<  $tr); } catch (Exception $_) { E(); }
    try { C($ks<= $tr); } catch (Exception $_) { E(); }
    try { C($ks > $tr); } catch (Exception $_) { E(); }
    try { C($ks >=$tr); } catch (Exception $_) { E(); }
    C($ks ==$tr);
    C($ks !=$tr);
    C($ks===$tr);
    C($ks!==$tr);
    try { I($ks<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($ks<  $fa); } catch (Exception $_) { E(); }
    try { C($ks<= $fa); } catch (Exception $_) { E(); }
    try { C($ks > $fa); } catch (Exception $_) { E(); }
    try { C($ks >=$fa); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($ks, $fa));
    C(HH\Lib\Legacy_FIXME\neq($ks, $fa));
    C($ks===$fa);
    C($ks!==$fa);
    try { I($ks<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($ks<  $nu); } catch (Exception $_) { E(); }
    try { C($ks<= $nu); } catch (Exception $_) { E(); }
    try { C($ks > $nu); } catch (Exception $_) { E(); }
    try { C($ks >=$nu); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($ks, $nu));
    C(HH\Lib\Legacy_FIXME\neq($ks, $nu));
    C($ks===$nu);
    C($ks!==$nu);
    try { I($ks<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[static] VAR ? $ks');
  begin_row('true');
    try { C($tr<  $ks); } catch (Exception $_) { E(); }
    try { C($tr<= $ks); } catch (Exception $_) { E(); }
    try { C($tr > $ks); } catch (Exception $_) { E(); }
    try { C($tr >=$ks); } catch (Exception $_) { E(); }
    C($tr ==$ks);
    C($tr !=$ks);
    C($tr===$ks);
    C($tr!==$ks);
    try { I($tr<=>$ks); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $ks); } catch (Exception $_) { E(); }
    try { C($fa<= $ks); } catch (Exception $_) { E(); }
    try { C($fa > $ks); } catch (Exception $_) { E(); }
    try { C($fa >=$ks); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($fa, $ks));
    C(HH\Lib\Legacy_FIXME\neq($fa, $ks));
    C($fa===$ks);
    C($fa!==$ks);
    try { I($fa<=>$ks); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $ks); } catch (Exception $_) { E(); }
    try { C($nu<= $ks); } catch (Exception $_) { E(); }
    try { C($nu > $ks); } catch (Exception $_) { E(); }
    try { C($nu >=$ks); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($nu, $ks));
    C(HH\Lib\Legacy_FIXME\neq($nu, $ks));
    C($nu===$ks);
    C($nu!==$ks);
    try { I($nu<=>$ks); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_keyset_empty_dynamic() :mixed{
  $ks = LV(keyset[]);
  $tr = LV(true);
  $fa = LV(false);
  $nu = LV(null);

  print_header('[dynamic] $ks ? VAR');
  begin_row('true');
    try { C($ks<  $tr); } catch (Exception $_) { E(); }
    try { C($ks<= $tr); } catch (Exception $_) { E(); }
    try { C($ks > $tr); } catch (Exception $_) { E(); }
    try { C($ks >=$tr); } catch (Exception $_) { E(); }
    C($ks ==$tr);
    C($ks !=$tr);
    C($ks===$tr);
    C($ks!==$tr);
    try { I($ks<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($ks<  $fa); } catch (Exception $_) { E(); }
    try { C($ks<= $fa); } catch (Exception $_) { E(); }
    try { C($ks > $fa); } catch (Exception $_) { E(); }
    try { C($ks >=$fa); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($ks, $fa));
    C(HH\Lib\Legacy_FIXME\neq($ks, $fa));
    C($ks===$fa);
    C($ks!==$fa);
    try { I($ks<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($ks<  $nu); } catch (Exception $_) { E(); }
    try { C($ks<= $nu); } catch (Exception $_) { E(); }
    try { C($ks > $nu); } catch (Exception $_) { E(); }
    try { C($ks >=$nu); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($ks, $nu));
    C(HH\Lib\Legacy_FIXME\neq($ks, $nu));
    C($ks===$nu);
    C($ks!==$nu);
    try { I($ks<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[dynamic] VAR ? $ks');
  begin_row('true');
    try { C($tr<  $ks); } catch (Exception $_) { E(); }
    try { C($tr<= $ks); } catch (Exception $_) { E(); }
    try { C($tr > $ks); } catch (Exception $_) { E(); }
    try { C($tr >=$ks); } catch (Exception $_) { E(); }
    C($tr ==$ks);
    C($tr !=$ks);
    C($tr===$ks);
    C($tr!==$ks);
    try { I($tr<=>$ks); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $ks); } catch (Exception $_) { E(); }
    try { C($fa<= $ks); } catch (Exception $_) { E(); }
    try { C($fa > $ks); } catch (Exception $_) { E(); }
    try { C($fa >=$ks); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($fa, $ks));
    C(HH\Lib\Legacy_FIXME\neq($fa, $ks));
    C($fa===$ks);
    C($fa!==$ks);
    try { I($fa<=>$ks); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $ks); } catch (Exception $_) { E(); }
    try { C($nu<= $ks); } catch (Exception $_) { E(); }
    try { C($nu > $ks); } catch (Exception $_) { E(); }
    try { C($nu >=$ks); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($nu, $ks));
    C(HH\Lib\Legacy_FIXME\neq($nu, $ks));
    C($nu===$ks);
    C($nu!==$ks);
    try { I($nu<=>$ks); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_keyset_nonempty_static() :mixed{
  $ky = keyset[42, 'foo'];
  $tr = true;
  $fa = false;
  $nu = null;

  print_header('[static] $ky ? VAR');
  begin_row('true');
    try { C($ky<  $tr); } catch (Exception $_) { E(); }
    try { C($ky<= $tr); } catch (Exception $_) { E(); }
    try { C($ky > $tr); } catch (Exception $_) { E(); }
    try { C($ky >=$tr); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($ky, $tr));
    C(HH\Lib\Legacy_FIXME\neq($ky, $tr));
    C($ky===$tr);
    C($ky!==$tr);
    try { I($ky<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($ky<  $fa); } catch (Exception $_) { E(); }
    try { C($ky<= $fa); } catch (Exception $_) { E(); }
    try { C($ky > $fa); } catch (Exception $_) { E(); }
    try { C($ky >=$fa); } catch (Exception $_) { E(); }
    C($ky ==$fa);
    C($ky !=$fa);
    C($ky===$fa);
    C($ky!==$fa);
    try { I($ky<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($ky<  $nu); } catch (Exception $_) { E(); }
    try { C($ky<= $nu); } catch (Exception $_) { E(); }
    try { C($ky > $nu); } catch (Exception $_) { E(); }
    try { C($ky >=$nu); } catch (Exception $_) { E(); }
    C($ky ==$nu);
    C($ky !=$nu);
    C($ky===$nu);
    C($ky!==$nu);
    try { I($ky<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[static] VAR ? $ky');
  begin_row('true');
    try { C($tr<  $ky); } catch (Exception $_) { E(); }
    try { C($tr<= $ky); } catch (Exception $_) { E(); }
    try { C($tr > $ky); } catch (Exception $_) { E(); }
    try { C($tr >=$ky); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($tr, $ky));
    C(HH\Lib\Legacy_FIXME\neq($tr, $ky));
    C($tr===$ky);
    C($tr!==$ky);
    try { I($tr<=>$ky); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $ky); } catch (Exception $_) { E(); }
    try { C($fa<= $ky); } catch (Exception $_) { E(); }
    try { C($fa > $ky); } catch (Exception $_) { E(); }
    try { C($fa >=$ky); } catch (Exception $_) { E(); }
    C($fa ==$ky);
    C($fa !=$ky);
    C($fa===$ky);
    C($fa!==$ky);
    try { I($fa<=>$ky); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $ky); } catch (Exception $_) { E(); }
    try { C($nu<= $ky); } catch (Exception $_) { E(); }
    try { C($nu > $ky); } catch (Exception $_) { E(); }
    try { C($nu >=$ky); } catch (Exception $_) { E(); }
    C($nu ==$ky);
    C($nu !=$ky);
    C($nu===$ky);
    C($nu!==$ky);
    try { I($nu<=>$ky); } catch (Exception $_) { E(); }
  print_footer();
}

<<__NEVER_INLINE>> function compare_keyset_nonempty_dynamic() :mixed{
  $ky = LV(keyset[42, 'foo']);
  $tr = LV(true);
  $fa = LV(false);
  $nu = LV(null);

  print_header('[dynamic] $ky ? VAR');
  begin_row('true');
    try { C($ky<  $tr); } catch (Exception $_) { E(); }
    try { C($ky<= $tr); } catch (Exception $_) { E(); }
    try { C($ky > $tr); } catch (Exception $_) { E(); }
    try { C($ky >=$tr); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($ky, $tr));
    C(HH\Lib\Legacy_FIXME\neq($ky, $tr));
    C($ky===$tr);
    C($ky!==$tr);
    try { I($ky<=>$tr); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($ky<  $fa); } catch (Exception $_) { E(); }
    try { C($ky<= $fa); } catch (Exception $_) { E(); }
    try { C($ky > $fa); } catch (Exception $_) { E(); }
    try { C($ky >=$fa); } catch (Exception $_) { E(); }
    C($ky ==$fa);
    C($ky !=$fa);
    C($ky===$fa);
    C($ky!==$fa);
    try { I($ky<=>$fa); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($ky<  $nu); } catch (Exception $_) { E(); }
    try { C($ky<= $nu); } catch (Exception $_) { E(); }
    try { C($ky > $nu); } catch (Exception $_) { E(); }
    try { C($ky >=$nu); } catch (Exception $_) { E(); }
    C($ky ==$nu);
    C($ky !=$nu);
    C($ky===$nu);
    C($ky!==$nu);
    try { I($ky<=>$nu); } catch (Exception $_) { E(); }
  print_footer();

  print_header('[dynamic] VAR ? $ky');
  begin_row('true');
    try { C($tr<  $ky); } catch (Exception $_) { E(); }
    try { C($tr<= $ky); } catch (Exception $_) { E(); }
    try { C($tr > $ky); } catch (Exception $_) { E(); }
    try { C($tr >=$ky); } catch (Exception $_) { E(); }
    C(HH\Lib\Legacy_FIXME\eq($tr, $ky));
    C(HH\Lib\Legacy_FIXME\neq($tr, $ky));
    C($tr===$ky);
    C($tr!==$ky);
    try { I($tr<=>$ky); } catch (Exception $_) { E(); }
  begin_row('false');
    try { C($fa<  $ky); } catch (Exception $_) { E(); }
    try { C($fa<= $ky); } catch (Exception $_) { E(); }
    try { C($fa > $ky); } catch (Exception $_) { E(); }
    try { C($fa >=$ky); } catch (Exception $_) { E(); }
    C($fa ==$ky);
    C($fa !=$ky);
    C($fa===$ky);
    C($fa!==$ky);
    try { I($fa<=>$ky); } catch (Exception $_) { E(); }
  begin_row('null');
    try { C($nu<  $ky); } catch (Exception $_) { E(); }
    try { C($nu<= $ky); } catch (Exception $_) { E(); }
    try { C($nu > $ky); } catch (Exception $_) { E(); }
    try { C($nu >=$ky); } catch (Exception $_) { E(); }
    C($nu ==$ky);
    C($nu !=$ky);
    C($nu===$ky);
    C($nu!==$ky);
    try { I($nu<=>$ky); } catch (Exception $_) { E(); }
  print_footer();
}

<<__EntryPoint>>
function main() :mixed{
  compare_varray_empty_static(); compare_varray_empty_dynamic();
  compare_varray_nonempty_static(); compare_varray_nonempty_dynamic();

  compare_darray_empty_static(); compare_darray_empty_dynamic();
  compare_darray_nonempty_static(); compare_darray_nonempty_dynamic();

  compare_vec_empty_static(); compare_vec_empty_dynamic();
  compare_vec_nonempty_static(); compare_vec_nonempty_dynamic();

  compare_dict_empty_static(); compare_dict_empty_dynamic();
  compare_dict_nonempty_static(); compare_dict_nonempty_dynamic();

  compare_keyset_empty_static(); compare_keyset_empty_dynamic();
  compare_keyset_nonempty_static(); compare_keyset_nonempty_dynamic();
}
