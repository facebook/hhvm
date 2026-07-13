<?hh

class Foo {}

function with_exn($fn): void {
  try {
    $fn();
    echo "\n";
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  preinc();
  postinc();
  predec();
  postdec();
}

function preinc(): void {
  echo 'preinc<';

  with_exn(() ==> {
    $l = null;
    ++$l; echo $l;
  });
  with_exn(() ==> {
    $l = false;
    ++$l; echo $l;
  });
  with_exn(() ==> {
    $l = true;
    ++$l; echo $l;
  });
  with_exn(() ==> {
    $l = 0;
    ++$l; echo $l;
  });
  with_exn(() ==> {
    $l = 42;
    ++$l; echo $l;
  });
  with_exn(() ==> {
    $l = 1.234;
    ++$l; echo $l;
  });
  with_exn(() ==> {
    $l = 'foobar';
    ++$l; echo $l;
  });
  with_exn(() ==> {
    $l = '';
    ++$l; echo $l;
  });
  with_exn(() ==> {
    $l = '1234';
    ++$l; echo $l;
  });
  with_exn(() ==> {
    $l = '1.234';
    ++$l; echo $l;
  });
  with_exn(() ==> {
    $l = HH\stdin();
    ++$l; echo $l;
  });
  echo ">\n";
}

function postinc(): void {
  echo 'postinc<';

  with_exn(() ==> {
    $l = null;
    $t = $l; $l++; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = false;
    $t = $l; $l++; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = true;
    $t = $l; $l++; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 0;
    $t = $l; $l++; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 42;
    $t = $l; $l++; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 1.234;
    $t = $l; $l++; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 'foobar';
    $t = $l; $l++; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = '';
    $t = $l; $l++; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = '1234';
    $t = $l; $l++; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = '1.234';
    $t = $l; $l++; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = HH\stdin();
    $t = $l; $l++; echo $t;
    echo "\n$l";
  });
  echo ">\n";
}


function predec(): void {
  echo 'predec<';

  with_exn(() ==> {
    $l = null;
    --$l; echo $l;
  });
  with_exn(() ==> {
    $l = false;
    --$l; echo $l;
  });
  with_exn(() ==> {
    $l = true;
    --$l; echo $l;
  });
  with_exn(() ==> {
    $l = 0;
    --$l; echo $l;
  });
  with_exn(() ==> {
    $l = 42;
    --$l; echo $l;
  });
  with_exn(() ==> {
    $l = 1.234;
    --$l; echo $l;
  });
  with_exn(() ==> {
    $l = 'foobar';
    --$l; echo $l;
  });
  with_exn(() ==> {
    $l = '';
    --$l; echo $l;
  });
  with_exn(() ==> {
    $l = '1234';
    --$l; echo $l;
  });
  with_exn(() ==> {
    $l = '1.234';
    --$l; echo $l;
  });
  with_exn(() ==> {
    $l = HH\stdin();
    --$l; echo $l;
  });
  echo ">\n";
}

function postdec(): void {
  echo 'postdec<';

  with_exn(() ==> {
    $l = null;
    $t = $l; $l--; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = false;
    $t = $l; $l--; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = true;
    $t = $l; $l--; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 0;
    $t = $l; $l--; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 42;
    $t = $l; $l--; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 1.234;
    $t = $l; $l--; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 'foobar';
    $t = $l; $l--; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = '';
    $t = $l; $l--; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = '1234';
    $t = $l; $l--; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = '1.234';
    $t = $l; $l--; echo $t;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = HH\stdin();
    $t = $l; $l--; echo $t;
    echo "\n$l";
  });

  echo ">\n";
}
