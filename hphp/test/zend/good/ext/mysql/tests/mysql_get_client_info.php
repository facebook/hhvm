<?hh

<<__EntryPoint>> function main(): void {
  if (!is_string($info = mysql_get_client_info()) || ('' === $info)) {
    printf("[001] Expecting string/any_non_empty, got %s/%s\n", gettype($info), $info);
  }

  try {
    mysql_get_client_info("too many arguments");
  } catch (Exception $e) {
    echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
  }

  print "done!";
}
