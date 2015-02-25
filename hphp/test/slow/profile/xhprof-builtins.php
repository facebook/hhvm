<?hh


function blug() {
  sleep(1);
}

xhprof_enable();
blug();
$stats = xhprof_disable();
if (isset($stats["blug==>sleep"])) {
  echo "ok without flag\n";
}


xhprof_enable(XHPROF_FLAGS_NO_BUILTINS);
blug();
$stats = xhprof_disable();
if (!isset($stats["blug==>sleep"])) {
  echo "ok with flag\n";
}
