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
    echo ++$l;
  });
  with_exn(() ==> {
    $l = false;
    echo ++$l;
  });
  with_exn(() ==> {
    $l = true;
    echo ++$l;
  });
  with_exn(() ==> {
    $l = 0;
    echo ++$l;
  });
  with_exn(() ==> {
    $l = 42;
    echo ++$l;
  });
  with_exn(() ==> {
    $l = 1.234;
    echo ++$l;
  });
  with_exn(() ==> {
    $l = 'foobar';
    echo ++$l;
  });
  with_exn(() ==> {
    $l = '';
    echo ++$l;
  });
  with_exn(() ==> {
    $l = '1234';
    echo ++$l;
  });
  with_exn(() ==> {
    $l = '1.234';
    echo ++$l;
  });
  with_exn(() ==> {
    $l = HH\stdin();
    echo ++$l;
  });
  echo ">\n";
}

function postinc(): void {
  echo 'postinc<';

  with_exn(() ==> {
    $l = null;
    echo $l++;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = false;
    echo $l++;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = true;
    echo $l++;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 0;
    echo $l++;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 42;
    echo $l++;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 1.234;
    echo $l++;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 'foobar';
    echo $l++;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = '';
    echo $l++;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = '1234';
    echo $l++;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = '1.234';
    echo $l++;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = HH\stdin();
    echo $l++;
    echo "\n$l";
  });
  echo ">\n";
}


function predec(): void {
  echo 'predec<';

  with_exn(() ==> {
    $l = null;
    echo --$l;
  });
  with_exn(() ==> {
    $l = false;
    echo --$l;
  });
  with_exn(() ==> {
    $l = true;
    echo --$l;
  });
  with_exn(() ==> {
    $l = 0;
    echo --$l;
  });
  with_exn(() ==> {
    $l = 42;
    echo --$l;
  });
  with_exn(() ==> {
    $l = 1.234;
    echo --$l;
  });
  with_exn(() ==> {
    $l = 'foobar';
    echo --$l;
  });
  with_exn(() ==> {
    $l = '';
    echo --$l;
  });
  with_exn(() ==> {
    $l = '1234';
    echo --$l;
  });
  with_exn(() ==> {
    $l = '1.234';
    echo --$l;
  });
  with_exn(() ==> {
    $l = HH\stdin();
    echo --$l;
  });
  echo ">\n";
}

function postdec(): void {
  echo 'postdec<';

  with_exn(() ==> {
    $l = null;
    echo $l--;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = false;
    echo $l--;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = true;
    echo $l--;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 0;
    echo $l--;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 42;
    echo $l--;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 1.234;
    echo $l--;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = 'foobar';
    echo $l--;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = '';
    echo $l--;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = '1234';
    echo $l--;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = '1.234';
    echo $l--;
    echo "\n$l";
  });
  with_exn(() ==> {
    $l = HH\stdin();
    echo $l--;
    echo "\n$l";
  });

  echo ">\n";
}
