<?php
/* foreach by reference has an interesting interaction with array_pop.
 * array_pop will reset the internal array pointer, but it will not reset the
 * foreach by reference array pointer; instead it will rewind it if the array
 * shrinks past the current position. This means that you can array_pop some
 * things off (including the current element), then append other things, and
 * iteration will resume at the first element that was added. This needs to
 * work even if the array grows before you start to array_pop, but _not_ if
 * there is a copy due to refcount != 1 before you start to array pop (in that
 * case iteration restarts from the internal array pointer, which array_pop
 * has reset). PHP is awesome like that.
 */
echo "---- packed no copy case\n";
$a = [1, 2, 3];
foreach ($a as &$v) {
  echo "$v\n";
  if ($v == 2) {
    $a[] = 4; // going from 3 => 4 elements should grow the array
    array_pop($a); array_pop($a); array_pop($a);
    $a[] = 9; $a[] = 10;
  }
}

echo "---- packed copy case\n";
$a = [1, 2, 3];
foreach ($a as &$v) {
  echo "$v\n";
  if ($v == 2) {
    $b = $a;
    $a[] = 4; // going from 3 => 4 elements should grow the array
    array_pop($a); array_pop($a); array_pop($a);
    $a[] = 9; $a[] = 10;
  }
}

echo "---- packed pop until totally empty case\n";
$a = [1, 2, 3];
foreach ($a as &$v) {
  echo "$v\n";
  if ($v == 2) {
    $a[] = 4; // going from 3 => 4 elements should grow the array
    array_pop($a); array_pop($a); array_pop($a); array_pop($a); array_pop($a);
    $a[] = 9; $a[] = 10;
  }
}

echo "---- packed was at end before pop case\n";
$a = [1, 2, 3];
foreach ($a as &$v) {
  echo "$v\n";
  if ($v == 3) {
    array_pop($a); array_pop($a);
    $a[] = 9; $a[] = 10;
  }
}

echo "---- mixed no copy case\n";
$a = ['a' => 1, 2, 3];
foreach ($a as &$v) {
  echo "$v\n";
  if ($v == 2) {
    $a[] = 4; // going from 3 => 4 elements should grow the array
    array_pop($a); array_pop($a); array_pop($a);
    $a[] = 9; $a[] = 10;
  }
}

echo "---- mixed copy case\n";
$a = ['a' => 1, 2, 3];
foreach ($a as &$v) {
  echo "$v\n";
  if ($v == 2) {
    $b = $a;
    $a[] = 4; // going from 3 => 4 elements should grow the array
    array_pop($a); array_pop($a); array_pop($a);
    $a[] = 9; $a[] = 10;
  }
}
