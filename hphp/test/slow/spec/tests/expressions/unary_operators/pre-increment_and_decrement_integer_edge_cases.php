<?hh

function incdec($x)
:mixed{
    echo "--- start incdec ---\n";
    $y = $x;

    var_dump($x);
    --$x;
    var_dump($x);
    --$x;
    var_dump($x);
    ++$x;
    var_dump($x);
    ++$x;
    var_dump($x);

// equivalent code using -=/+= instead of --/++.

    var_dump($y);
    $y -= 1;
    var_dump($y);
    $y -= 1;
    var_dump($y);
    $y += 1;
    var_dump($y);
    $y += 1;
    var_dump($y);
    echo "--- end incdec ---\n";
}

function incdecrev($x)
:mixed{
    echo "--- start incdecrev ---\n";
    $y = $x;

    var_dump($x);
    ++$x;
    var_dump($x);
    ++$x;
    var_dump($x);
    --$x;
    var_dump($x);
    --$x;
    var_dump($x);

// equivalent code using -=/+= instead of --/++.

    var_dump($y);
    $y += 1;
    var_dump($y);
    $y += 1;
    var_dump($y);
    $y -= 1;
    var_dump($y);
    $y -= 1;
    var_dump($y);
    echo "--- end incdecrev ---\n";
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  $i32 = 1 << 31; // if this is negative, we have a 32-bit int
  $i64 = 1 << 63; // same as $i32 for 32-bit int; otherwise, is 64-bit
  $IntMin = ($i32 < 0) ? $i32 : $i64;
  $IntMax = ~$IntMin;

  incdec($IntMin);
  incdecrev($IntMax);
}
