<?hh

function check_trusted_bool(bool $b): void {
  if ($b) { // no error expected
  }
}

function check_like_bool(HH\Lib\Ref<bool> $b): void {
  if ($b->value) { // no error expected
  }
}

function check_dynamic(dynamic $dyn): void {
  if ($dyn) { // no error expected
  }
}
