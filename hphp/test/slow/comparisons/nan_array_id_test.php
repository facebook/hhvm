<?hh

function test_pair($k1, $v1, $k2, $v2) :mixed{
  echo "$k1 cmp $k2:\n";
  try {
    echo (($v1 === $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 !== $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 < $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 <= $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 == $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 != $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 >= $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 > $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    $cmp = $v1 <=> $v2;
    echo "$cmp";
  } catch (Exception $e) {
    echo "Err";
  }

  echo "\n";
}

<<__EntryPoint>>
function main(): void {
  $make_nan_array = () ==> vec[1, NAN];
  $arr1 = $make_nan_array();
  $arr2 = $make_nan_array();
  $arr3 = vec[NAN, 1];
  $arr4 = vec[1, NAN, 2];

  $arr = dict['array arr1' => $arr1, 'array arr2' => $arr2,
                'array arr3' => $arr3, 'array arr4' => $arr4];

  foreach ($arr as $k1 => $v1) {
    foreach ($arr as $k2 => $v2) {
      test_pair($k1, $v1, $k2, $v2);
    }
  }
}
