<?hh
<<__EntryPoint>> function main(): void {
if (PHP_OS === 'Linux') {
  $GLOBALS['USE_UNIX_SOCKET'] = true;
  require(__DIR__ . '/context.php');
} else {
  // This test (and the socket configuration it is testing)
  // is not supported on other platforms. Just skip it.
  echo "OK!\n";
}
}
