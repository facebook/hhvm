<?hh


<<__EntryPoint>>
function main_double_json_corruption() :mixed{
  $vals = vec[
    1.4142135623730951,
    -9.223372036854776e18,
    1.0715086071862673e+301,
    1.8665272370064378e-301,
  ];

  foreach ($vals as $val) {
    // encoding with default options uses 14 decimals and loses precision
    $error = null;
    $jval = json_encode_with_error($val, inout $error);
    var_dump($val == json_decode($jval));

    // encoding with JSON_FB_FULL_FLOAT_PRECISION uses 17 decimals and retains precision
    $error = null;
    $jval = json_encode_with_error($val, inout $error, JSON_FB_FULL_FLOAT_PRECISION);
    var_dump($val == json_decode($jval));
  }
}
