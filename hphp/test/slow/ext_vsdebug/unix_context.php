<?hh
<<__EntryPoint>> function main1(): void {
if (PHP_OS === 'Linux') {
  \HH\global_set('USE_UNIX_SOCKET', true);
  require(__DIR__ . '/context.php');
  main();
} else {
  // This test (and the socket configuration it is testing)
  // is not supported on other platforms. Just skip it.
  echo "OK!\n";
}
}
