<?hh

type example_shape = shape(
  ?'example_1' => int,
  ?'example_2' => int,
  ?'example_3' => int,
  ?'example_4' => int,
  ?'example_5' => int,
  ?'example_6' => int,
  ?'example_7' => int,
  ?'example_8' => int,
  ?'example_9' => int,
  ?'example_10' => int,
  ?'example_11' => int,
  ?'example_12' => int,
  ?'example_13' => int,
  ?'example_14' => int,
  ?'example_15' => int,
);
function takes_example_shape(example_shape $in): string {

  $bits = vec[];

  if (Shapes::keyExists($in, 'example_1')) {
    $bits[] = 'example_1';
  }
  if (Shapes::keyExists($in, 'example_2')) {
    $bits[] = 'example_2';
  }
  if (Shapes::keyExists($in, 'example_3')) {
    $bits[] = 'example_3';
  }
  if (Shapes::keyExists($in, 'example_4')) {
    $bits[] = 'example_4';
  }
  if (Shapes::keyExists($in, 'example_5')) {
    $bits[] = 'example_5';
  }
  if (Shapes::keyExists($in, 'example_6')) {
    $bits[] = 'example_6';
  }
  if (Shapes::keyExists($in, 'example_7')) {
    $bits[] = 'example_7';
  }
  if (Shapes::keyExists($in, 'example_8')) {
    $bits[] = 'example_8';
  }
  if (Shapes::keyExists($in, 'example_9')) {
    $bits[] = 'example_9';
  }
  if (Shapes::keyExists($in, 'example_10')) {
    $bits[] = 'example_10';
  }
  if (Shapes::keyExists($in, 'example_6')) {
    $bits[] = 'example_6';
  }
  if (Shapes::keyExists($in, 'example_7')) {
    $bits[] = 'example_7';
  }
  if (Shapes::keyExists($in, 'example_8')) {
    $bits[] = 'example_8';
  }
  if (Shapes::keyExists($in, 'example_9')) {
    $bits[] = 'example_9';
  }
  if (Shapes::keyExists($in, 'example_10')) {
    $bits[] = 'example_10';
  }
  if (Shapes::keyExists($in, 'example_11')) {
    $bits[] = 'example_11';
  }
  if (Shapes::keyExists($in, 'example_12')) {
    $bits[] = 'example_12';
  }
  if (Shapes::keyExists($in, 'example_13')) {
    $bits[] = 'example_13';
  }
  if (Shapes::keyExists($in, 'example_14')) {
    $bits[] = 'example_14';
  }
  if (Shapes::keyExists($in, 'example_15')) {
    $bits[] = 'example_15';
  }

  return implode(',', $bits);
}
