<?hh

class Foo {}

<<__EntryPoint>>
function main(): void {
  preinc();
  postinc();
  predec();
  postdec();
}

function preinc(): void {
  echo 'preinc<';

  $l = null;
  echo ++$l;
  $l = false;
  echo ++$l;
  $l = true;
  echo ++$l;
  $l = 0;
  echo ++$l;
  $l = 42;
  echo ++$l;
  $l = 1.234;
  echo ++$l;
  $l = 'foobar';
  echo ++$l;
  $l = '';
  echo ++$l;
  $l = '1234';
  echo ++$l;
  $l = '1.234';
  echo ++$l;
  $l = STDIN;
  echo ++$l;

  echo ">\n";
}

function postinc(): void {
  echo 'postinc<';

  $l = null;
  echo $l++;
  echo $l;
  $l = false;
  echo $l++;
  echo $l;
  $l = true;
  echo $l++;
  echo $l;
  $l = 0;
  echo $l++;
  echo $l;
  $l = 42;
  echo $l++;
  echo $l;
  $l = 1.234;
  echo $l++;
  echo $l;
  $l = 'foobar';
  echo $l++;
  echo $l;
  $l = '';
  echo $l++;
  echo $l;
  $l = '1234';
  echo $l++;
  echo $l;
  $l = '1.234';
  echo $l++;
  echo $l;
  $l = STDIN;
  echo $l++;
  echo $l;

  echo ">\n";
}


function predec(): void {
  echo 'predec<';

  $l = null;
  echo --$l;
  $l = false;
  echo --$l;
  $l = true;
  echo --$l;
  $l = 0;
  echo --$l;
  $l = 42;
  echo --$l;
  $l = 1.234;
  echo --$l;
  $l = 'foobar';
  echo --$l;
  $l = '';
  echo --$l;
  $l = '1234';
  echo --$l;
  $l = '1.234';
  echo --$l;
  $l = STDIN;
  echo --$l;

  echo ">\n";
}

function postdec(): void {
  echo 'postdec<';

  $l = null;
  echo $l--;
  echo $l;
  $l = false;
  echo $l--;
  echo $l;
  $l = true;
  echo $l--;
  echo $l;
  $l = 0;
  echo $l--;
  echo $l;
  $l = 42;
  echo $l--;
  echo $l;
  $l = 1.234;
  echo $l--;
  echo $l;
  $l = 'foobar';
  echo $l--;
  echo $l;
  $l = '';
  echo $l--;
  echo $l;
  $l = '1234';
  echo $l--;
  echo $l;
  $l = '1.234';
  echo $l--;
  echo $l;
  $l = STDIN;
  echo $l--;
  echo $l;

  echo ">\n";
}
