<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

class C {
  function f1($a, $b) :mixed{
    return $a + $b;
  }

  function f2($a) :mixed{
    return $a + 1;
  }

  // Note: we have a bug in hackir if we remove the $this's below
  function test($a) :mixed{
    $x = $this->f1($this->f2($a), $a);
    echo $x;
    echo "\n";
  }
}

//$x = new C();
//$x->test(1);

function f3($a, $b) :mixed{
  return $a + $b;
}

function f4($a) :mixed{
  return $a + 1;
}

/* Note: we have a bug in hackir if we remove the $this's below */
function test($a, $f1, $f2) :mixed{
  $x = $f1($f2($a), $a);
  echo $x;
  echo "\n";
}
<<__EntryPoint>> function main(): void {
test(1, f3<>, f4<>);
}
