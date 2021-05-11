<?hh

<<__EntryPoint>>
function main(): void {
  $s_string = '1111111111';
  $s_search = '/1/';
  $s_replace = 'One ';
  $i_limit = 1;
  $i_count = 0;

  $error = null;
  $s_output = preg_replace_with_count_and_error(
    $s_search,
    $s_replace,
    $s_string,
    $i_limit,
    inout $i_count,
    inout $error,
  );
  echo "Output = " . var_export($s_output, True) . "\n";
  echo "Count  = $i_count\n";
  var_dump($error === null);

  $i_limit = strlen($s_string);
  $error = null;
  $s_output = preg_replace_with_count_and_error(
    $s_search,
    $s_replace,
    $s_string,
    $i_limit,
    inout $i_count,
    inout $error,
  );
  echo "Output = " . var_export($s_output, True) . "\n";
  echo "Count  = $i_count\n";
  var_dump($error === null);
}
