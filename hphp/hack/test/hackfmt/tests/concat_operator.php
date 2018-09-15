<?hh

$foo = 'bar' . $baz;

$str = 'short string'.$very_long_variable_name_which_probably_ought_to_be_shortened;

$str = 'not as short string'.$very_long_variable_name_which_probably_ought_to_be_shortened;

$str = $first
  . $second
  . $third
  . $fourth
  . $fifth
  . $sixth
  . $seventh
  . $eighth
  . $ninth
  . $tenth
  . $eleventh
  . $twelfth;


$str = $one . $two + $three;

function_call(
  '/' . $regex . '/S',
  $long_argument_triggering_line_breaks_for_argument_list,
);
