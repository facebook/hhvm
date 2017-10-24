<?hh

function fuzz(inout $a) {
  $orig_a = $a;
  $a = 'fuzzed';
  return array(
    $orig_a,
    $a,
    array(
      debug_backtrace()[0]['function'],
      debug_backtrace()[1]['function'],
    ),
  );
}

function main() {
  $arr = array();
  $arr[0][12][]['bar']['x'] = 'value1';
  $arr[18][]['hello']['y'] = 'value2';

  list(
    $original,
    $new,
    list(
      $callee,
      $caller,
    ),
  ) = fuzz(inout $arr[0][12][0]['bar']['x']);

  var_dump($original, $new, $callee, $caller, $arr);

  list(
    $original,
    $new,
    list(
      $callee,
      $caller,
    ),
  ) = fuzz(inout $arr[18][0]['hello']['y']);
  var_dump($original, $new, $callee, $caller, $arr);

  $arr[0][12][0]['bar']['x'] = 'value1';
  $arr[18][0]['hello']['y'] = 'value2';

  list(
    $original,
    $new,
    list(
      $callee,
      $caller,
    ),
  ) = fuzz($arr[0][12][0]['bar']['x']);

  var_dump($original, $new, $callee, $caller, $arr);

  list(
    $original,
    $new,
    list(
      $callee,
      $caller,
    ),
  ) = fuzz($arr[18][0]['hello']['y']);
  var_dump($original, $new, $callee, $caller, $arr);

  $a = array(Vector{1, 2});
  list(
    $original,
    $new,
    list(
      $callee,
      $caller,
    ),
  ) = fuzz(inout $a[0]);
  var_dump($original, $new, $callee, $caller, $a);

  $a = array(Vector{1, 2});
  list(
    $original,
    $new,
    list(
      $callee,
      $caller,
    ),
  ) = fuzz(inout $a);
  var_dump($original, $new, $callee, $caller, $a);

  $a = array(vec[Vector{1, 2}]);
  list(
    $original,
    $new,
    list(
      $callee,
      $caller,
    ),
  ) = fuzz(inout $a[0][0]);
  var_dump($original, $new, $callee, $caller, $a);
}

main();
