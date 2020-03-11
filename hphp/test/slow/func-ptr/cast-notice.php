<?hh

function foobar() {}

function LV($x) { return __hhvm_intrinsics\launder_value($x); }
class Info { public static string $error = ''; }
function handle_error($_errno, $msg, ...) {
  if (Info::$error !== '') return false;
  Info::$error = $msg;
  return true;
}
<<__NEVER_INLINE>> function print_header($title) {
  echo "$title\n";
  echo "+-----------------+----------------------+--------------------------------+\n";
  echo "|                 | Result               | Error?                         |\n";
  echo "+=================+======================+================================+";
}
<<__NEVER_INLINE>> function begin_row($expr) {
  if (Info::$error !== '') throw new Exception(Info::$error);
  printf("\n| %-15s |", $expr);
}
<<__NEVER_INLINE>> function C($f) {
  ob_start(); $f(); $s = trim(ob_get_clean()); ob_end_flush();
  printf(" %-20s | %-30s |", $s ?: '*** FAIL', Info::$error);
  Info::$error = '';
}
<<__NEVER_INLINE>> function X(mixed $v) {
  C(() ==> { var_dump($v); });
}
<<__NEVER_INLINE>> function print_footer() {
  echo "\n+-----------------+----------------------+--------------------------------+\n\n";
}

<<__EntryPoint>>
function main() {
  set_error_handler('handle_error');

  $s = 'foobar';
  print_header('[static] $s = \'foobar\'');
  begin_row('(bool)$s')        ;X((bool)$s);
  begin_row('(int)$s')         ;X((int)$s);
  begin_row('(float)$s')       ;X((float)$s);
  begin_row('(string)$s')      ;X((string)$s);
  begin_row('$s == true')      ;X($s == true);
  begin_row('true == $s')      ;X(true == $s);
  begin_row('$s == false')     ;X($s == false);
  begin_row('false == $s')     ;X(false == $s);
  begin_row('!$s')             ;X(!$s);
  begin_row('!!$s')            ;X(!!$s);
  begin_row('if ($s) ...')     ;C(() ==> { if ($s) echo "ok!"; });
  begin_row('switch ($s) ...') ;C(() ==> { switch ($s) { case true: echo "ok!"; default: break; } });
  print_footer();

  $s = LV($s);
  print_header('[dynamic] $s = LV(\'foobar\')');
  begin_row('(bool)$s')        ;X((bool)$s);
  begin_row('(int)$s')         ;X((int)$s);
  begin_row('(float)$s')       ;X((float)$s);
  begin_row('(string)$s')      ;X((string)$s);
  begin_row('$s == true')      ;X($s == true);
  begin_row('true == $s')      ;X(true == $s);
  begin_row('$s == false')     ;X($s == false);
  begin_row('false == $s')     ;X(false == $s);
  begin_row('!$s')             ;X(!$s);
  begin_row('!!$s')            ;X(!!$s);
  begin_row('if ($s) ...')     ;C(() ==> { if ($s) echo "ok!"; });
  begin_row('switch ($s) ...') ;C(() ==> { switch ($s) { case true: echo "ok!"; default: break; } });
  print_footer();
  unset($s);

  $f = fun('foobar');
  print_header('[static] $f = fun(\'foobar\')');
  begin_row('(bool)$f')        ;X((bool)$f);
  begin_row('(int)$f')         ;X((int)$f);
  begin_row('(float)$f')       ;X((float)$f);
  begin_row('(string)$f')      ;X((string)$f);
  begin_row('$f == true')      ;X($f == true);
  begin_row('true == $f')      ;X(true == $f);
  begin_row('$f == false')     ;X($f == false);
  begin_row('false == $f')     ;X(false == $f);
  begin_row('!$f')             ;X(!$f);
  begin_row('!!$f')            ;X(!!$f);
  begin_row('if ($f) ...')     ;C(() ==> { if ($f) echo "ok!"; });
  begin_row('switch ($f) ...') ;C(() ==> { switch ($f) { case true: echo "ok!"; default: break; } });
  print_footer();

  $f = LV($f);
  print_header('[dynamic] $f = LV(fun(\'foobar\'))');
  begin_row('(bool)$f')        ;X((bool)$f);
  begin_row('(int)$f')         ;X((int)$f);
  begin_row('(float)$f')       ;X((float)$f);
  begin_row('(string)$f')      ;X((string)$f);
  begin_row('$f == true')      ;X($f == true);
  begin_row('true == $f')      ;X(true == $f);
  begin_row('$f == false')     ;X($f == false);
  begin_row('false == $f')     ;X(false == $f);
  begin_row('!$f')             ;X(!$f);
  begin_row('!!$f')            ;X(!!$f);
  begin_row('if ($f) ...')     ;C(() ==> { if ($f) echo "ok!"; });
  begin_row('switch ($f) ...') ;C(() ==> { switch ($f) { case true: echo "ok!"; default: break; } });
  print_footer();
  unset($f);
}
