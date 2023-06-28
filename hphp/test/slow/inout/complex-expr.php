<?hh

function fuzz(inout $a) :mixed{
  $orig_a = $a;
  $a = 'fuzzed';
  return varray[
    $orig_a,
    $a,
    varray[
      debug_backtrace()[0]['function'],
      debug_backtrace()[1]['function'],
    ],
  ];
}

<<__EntryPoint>>
function main() :mixed{
  $arr = darray[0 => darray[12 => varray[darray['bar' => darray[]]]],
          18 => varray[darray['hello' => darray[]]]];
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

  $a = varray[Vector{1, 2}];
  list(
    $original,
    $new,
    list(
      $callee,
      $caller,
    ),
  ) = fuzz(inout $a[0]);
  var_dump($original, $new, $callee, $caller, $a);

  $a = varray[Vector{1, 2}];
  list(
    $original,
    $new,
    list(
      $callee,
      $caller,
    ),
  ) = fuzz(inout $a);
  var_dump($original, $new, $callee, $caller, $a);

  $a = varray[vec[Vector{1, 2}]];
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
