<?hh

<<__DynamicallyCallable>>
function do_prints($from, $fatal) {
  $fstr = $fatal ? "true" : "false";
  $f = __FUNCTION__."($from, $fstr) >";
  $out = fopen('php://stdout', 'w');

  echo "$f echo\n";
  var_dump("$f var_dump");
  error_log("$f error_log");
  fprintf(\HH\stdout(), "$f fprintf stdout\n");
  fprintf(\HH\stderr(), "$f fprintf stderr\n");
  fprintf($out, "$f fprintf \$out\n");
  file_put_contents('php://stdout', "$f file_put_contents stdout\n");

  trigger_error("$f trigger_error notice", E_USER_NOTICE);
  if ($fatal) {
    i_dont_exist(); // !
  }
}

<<__EntryPoint>>
function main() {
  $ctx = HH\execution_context();
  echo "In $ctx\n";

  do_prints($ctx, false);

  if (HH\execution_context() !== "xbox") {
    $r = fb_call_user_func_async(__FILE__, "do_prints", "xbox-func", true);
    try {
      fb_end_user_func_async($r);
    } catch (Exception $exn) {
      echo "xbox thread threw: ".trim($exn->getMessage())."\n";
    }
  }

  echo "$ctx done.\n";
}
