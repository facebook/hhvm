<?hh

require_once('test_base.inc');

function opts(bool $hack_arr = false): string {
  $args = array(
    '-vEval.HackArrCompatTypeHintNotices=1',
    '-vEval.EnableHipHopSyntax=1',
  );
  if ($hack_arr) {
    $args[] = '-vEval.HackArrDVArrs=true';
  }
  return implode(' ', $args);
}

function hphp_opts(bool $hack_arr = false): string {
  $args = array(
    '-vRuntime.Eval.HackArrCompatTypeHintNotices=1',
    '-vEnableHipHopSyntax=1',
  );
  if ($hack_arr) {
    $args[] = '-vHackArrDVArrs=1';
  }
  return implode(' ', $args);
}

<<__EntryPoint>> function main() {
  requestAll(
    array(
      array(
        'test_get_headers_secure.php',
        null,
        array(
          'xyzzy' => 42,
          'XyZZy' => 43,
          'XYZZY' => 44,
          'xxxxx' => 45,
        )
      )
    ),
    opts(),
    hphp_opts(),
  );
  requestAll(
    array(
      array(
        'test_get_headers_secure.php',
        null,
        array(
          'xyzzy' => 42,
          'XyZZy' => 43,
          'XYZZY' => 44,
          'xxxxx' => 45,
        )
      )
    ),
    opts(true),
    hphp_opts(true),
  );
}
