<?hh

function fuzz(inout $a) :mixed{
  $orig_a = $a;
  $a = 'fuzzed';
  return vec[
    $orig_a,
    $a,
    vec[
      debug_backtrace()[0]['function'],
      debug_backtrace()[1]['function'],
    ],
  ];
}

<<__EntryPoint>>
function main() :mixed{
  $arr = dict[0 => dict[12 => vec[dict['bar' => dict[]]]],
          18 => vec[dict['hello' => dict[]]]];
  $arr[0][12][0]['bar']['x'] = 'value1';
  $arr[18][0]['hello']['y'] = 'value2';

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

  $a = vec[Vector{1, 2}];
  list(
    $original,
    $new,
    list(
      $callee,
      $caller,
    ),
  ) = fuzz(inout $a[0]);
  var_dump($original, $new, $callee, $caller, $a);

  $a = vec[Vector{1, 2}];
  list(
    $original,
    $new,
    list(
      $callee,
      $caller,
    ),
  ) = fuzz(inout $a);
  var_dump($original, $new, $callee, $caller, $a);

  $a = vec[vec[Vector{1, 2}]];
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
