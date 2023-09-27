<?hh

function expect_exception((function(): void) $f, string $msg): void {
  try {
    $f();
  } catch (Exception $e) {
    $got = $e->getMessage();
    if ($msg !== $got) {
      throw new Exception("Exception caught but mismatched message:\nGot:      $got\nExpected: $msg");
    }
    return;
  }
  throw new Exception("Exception expected but not thrown");
}

function handleError($what, string $msg, $_, $_, $_): void {
  if ($what === E_NOTICE) {
    fprintf(HH\stderr(), "NOTICE: %s\n", $msg);
    return;
  }

  throw new Exception($msg);
}

function throw_errors(): void {
  set_error_handler(handleError<>);
}
