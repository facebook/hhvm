<?hh

<<__EntryPoint>>
function main(): void {
  $resource = fopen(__FILE__, "r");

  var_dump($resource);

  $error = null;
  $result = json_encode_with_error($resource, inout $error);
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');

  $error = null;
  $result = json_encode_with_error(
    $resource,
    inout $error,
    JSON_PARTIAL_OUTPUT_ON_ERROR,
  );
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');
}
