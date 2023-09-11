<?hh

function check(string $input): void {
  echo var_export($input, true)." ";
  $error_message = preg_get_error_message_if_invalid($input);
  $is_valid = $error_message is null;
  echo $is_valid ? "valid" : "invalid";
  echo " ".($error_message ?? 'NULL');
  echo "\n";
}

<<__EntryPoint>>
function main(): void {
  check('/ok/');
  check('/o');
  check('');
  check('aa');
  check('//'.chr(0).'has null byte');
}
