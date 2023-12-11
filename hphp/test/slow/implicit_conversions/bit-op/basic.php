<?hh


function with_exn($fn): mixed {
  try {
    return $fn();
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
    return null;
  }
}

<<__EntryPoint>>
function main(): void {
  $vals = vec[
    0,
    -10,
    1.234,
    INF,
    NAN,
    true,
    false,
    null,
    HH\stdin(),
    "string",
    vec[42],
    dict['foobar' => false],
  ];
  foreach($vals as $i) {
    if ($i is float || $i is int) {
      echo '$i '; var_dump($i);
      echo 'not<';
      with_exn(() ==> {~$i;}); // this should still throw as applicable
      echo with_exn(() ==> ~$i);
      echo ">\n";
    }
    foreach($vals as $j) {
      echo '$i '; var_dump($i); echo '$j '; var_dump($j);
      and($i, $j);
      or($i, $j);
      xor($i, $j);
      shl($i, $j);
      shr($i, $j);
    }
  }
}

function and(mixed $i, mixed $j): void {
  echo 'and<';
  with_exn(() ==> { $i & $j; }); // this should still throw as applicable
  echo with_exn(() ==> { $i &= $j; return $i; });
  echo ">\n";
}

function or(mixed $i, mixed $j): void {
  echo 'or<';
  with_exn(() ==> { $i | $j; }); // this should still throw as applicable
  echo with_exn(() ==> { $i |= $j; return $i; });
  echo ">\n";
}

function xor(mixed $i, mixed $j): void {
  echo 'xor<';
  with_exn(() ==> { $i ^ $j; }); // this should still throw as applicable
  echo with_exn(() ==> { $i ^= $j; return $i; });
  echo ">\n";
}

function shl(mixed $i, mixed $j): void {
  echo 'shl<';
  with_exn(() ==> { $i << $j; }); // this should still throw as applicable
  echo with_exn(() ==> { $i <<= $j; return $i; });
  echo ">\n";
}

function shr(mixed $i, mixed $j): void {
  echo 'shr<';
  with_exn(() ==> { $i >> $j; }); // this should still throw as applicable
  echo with_exn(() ==> { $i >>= $j; return $i; });
  echo ">\n";
}
