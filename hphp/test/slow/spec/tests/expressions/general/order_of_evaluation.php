<?hh

///*
function f($a, $b)
:mixed{
    echo "Inside f: \$a = $a, \$b = $b\n";
    return 22;
}

function g($a)
:mixed{
    echo "Inside g: \$a = $a\n";
    return 10;
}

function f1($a)
:mixed{
    echo "Inside f1\n";
    return $a;
}

function f2($a)
:mixed{
    echo "Inside f2\n";
    return $a;
}

function f3($a)
:mixed{
    echo "Inside f3\n";
    return $a;
}

function f4($a)
:mixed{
    echo "Inside f4\n";
    return $a;
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);
  //*/

  ///*
  $i = 10;
  echo "\$i = $i: "; $t = $i; $i++; $e = $t + $i; echo '$i++ + $i = '.$e."\t\t"; echo "\$i = $i\n";
  $i = 10;
  echo "\$i = $i: "; $t = $i; $i--; $e = $i - $t; echo '$i - $i-- = '.$e."\t\t"; echo "\$i = $i\n";
  $i = 10;
  echo "\$i = $i: "; ++$i; $e = $i * $i; echo '$++i * $i = '.$e."\t"; echo "\$i = $i\n";
  $i = 10;
  echo "\$i = $i: "; --$i; $e = $i / $i; echo '$i / --$i = '.$e."\t\t"; echo "\$i = $i\n";
  var_dump(10/9);
  //*/

  ///*
  $values = vec[0, 1, 2, 3, 4, 5, 6];
  $i = 3;
  echo "\n".'initially, $i = '.$i."\n";
  ++$i; $rhs = $values[$i]; $values[$i] = $rhs;        // prefix ++ (rhs before lval target)
  echo '  finally, $i = '.$i."\n";
  foreach ($values as $a)
  {
      echo "$a ";
  }
  echo "\n";

  $values = vec[0, 1, 2, 3, 4, 5, 6];
  $i = 3;
  echo "\n".'initially, $i = '.$i."\n";
  $t = $i; $i++; $rhs = $values[$t]; $values[$i] = $rhs;        // postfix ++ (rhs before lval target)
  echo '  finally, $i = '.$i."\n";
  foreach ($values as $a)
  {
      echo "$a ";
  }
  echo "\n";
  //*/

  ///*
  $values = vec[0, 1, 2, 3, 4, 5, 6];
  $i = 3;
  echo "\n".'initially, $i = '.$i."\n";
  $a = $i; ++$i; $b = $i; $rhs = f($a, $b); $values[$i] = $rhs;
  echo '  finally, $i = '.$i."\n";
  foreach ($values as $a)
  {
      echo "$a ";
  }
  echo "\n";

  $values = vec[0, 1, 2, 3, 4, 5, 6];
  $i = 3;
  echo "\n".'initially, $i = '.$i."\n";
  $a = $i; ++$i; $b = $i; $rhs = f($a, $b); $values[$i] = $values[$i] * $rhs;
  echo '  finally, $i = '.$i."\n";
  foreach ($values as $a)
  {
      echo "$a ";
  }
  echo "\n";

  $values = vec[0, 1, 2, 3, 4, 5, 6];
  $i = 3;
  echo "\n".'initially, $i = '.$i."\n";
  $a = $i; ++$i; $b = $i; $lft = f($a, $b); ++$i; $rgt = g($i); $rhs = $lft + $rgt; $values[$i] = $rhs;
  echo '  finally, $i = '.$i."\n";
  foreach ($values as $a)
  {
      echo "$a ";
  }
  echo "\n";

  $values = vec[0, 1, 2, 3, 4, 5, 6];
  $i = 3;
  echo "\n".'initially, $i = '.$i."\n";
  $a = $i; ++$i; $b = $i; $lft = f($a, $b); ++$i; $rgt = g($i); $rhs = $lft - $rgt; $values[$i] = $rhs;
  echo '  finally, $i = '.$i."\n";
  foreach ($values as $a)
  {
      echo "$a ";
  }
  echo "\n";

  $values = vec[0, 1, 2, 3, 4, 5, 6];
  $i = 3;
  echo "\n".'initially, $i = '.$i."\n";
  $a1 = $i; ++$i; $b1 = $i; $t1 = f($a1, $b1); ++$i; $t2 = g($i); $quot = $t1 / $t2; --$i; $a2 = $i; $b2 = $i; $t3 = f($a2, $b2); $rhs = $quot - $t3; $values[$i] = $rhs;
  echo '  finally, $i = '.$i."\n";
  foreach ($values as $a)
  {
      echo (string)$a." ";
  }
  echo "\n";

  $values = vec[0, 1, 2, 3, 4, 5, 6];
  $i = 3;
  echo "\n".'initially, $i = '.$i."\n";
  $a1 = $i; ++$i; $b1 = $i; $t1 = f($a1, $b1); ++$i; $t2 = g($i); --$i; $a2 = $i; $b2 = $i; $t3 = f($a2, $b2); $i = 5; $t4 = $i; $rhs = $t1 + $t2 + $t3 + $t4; $values[$i] = $rhs;
  echo '  finally, $i = '.$i."\n";
  foreach ($values as $a)
  {
      echo "$a ";
  }
  echo "\n";
  //*/

  echo 'f(10, 12) + g(15) = '.(f(10, 12) + g(15))."\n";
  echo 'f(10, 12) - g(15) = '.(f(10, 12) - g(15))."\n";
  echo 'f(10, 12) * g(15) = '.(f(10, 12) * g(15))."\n";
  echo 'f(10, 12) / g(15) = '.(string)(f(10, 12) / g(15))."\n";

  $values = vec[0, 1, 2, 3, 4, 5, 6];
  var_dump($values);
  $values[f1(4) - f2(2)] = $values[f3(3) * f4(2)];
  var_dump($values);
  $values = vec[0, 1, 2, 3, 4, 5, 6];
  $values[f1(1) + f2(2)] = $values[f3(6) / f4(3)];
  var_dump($values);
}
