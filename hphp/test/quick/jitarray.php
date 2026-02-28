<?hh

function f() :mixed{ return dict[]; }

function populateArray($max) :mixed{
  $a = f();
  for ($i = 0; $i < $max; ++$i) {
    $a[$i] = $i * 9;
  }

  for ($i = 0; $i < $max; ++$i) {
    if ($a[$i] != $i * 9) {
      echo "mismatch: "; echo $i; echo ": ";
      echo $a[$i]; echo "; should be "; echo $i * 9;
      echo "\n";
    }
  }
}

<<__EntryPoint>> function main(): void {
  $a = f();
  echo "writing: "; echo ($a[1] = 66); echo "\n";
  echo "reading: "; echo $a[1]; echo "\n";

  populateArray(1000);
}
