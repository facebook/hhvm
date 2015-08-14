<?hh // strict

class Cns {
  const X1 = "\x00\x00\x00\x00";
  const X2 = "\x70\x00\xff\x00";
  const X3 = "\a\b\c\d\e\f\g\h\i\j\k\l\m\n\o\p\q\r\s\t\u\v\w\x\y\z";
  const X4 = "\$";
}

function dump(string $s): void {
  $a = array();
  /* HH_FIXME[2049] */
  /* HH_FIXME[4107] */
  for ($i = 0; $i < strlen($s); $i++) {
    /* HH_FIXME[2049] */
    /* HH_FIXME[4107] */
    $a[] = ord($s[$i]);
  }
  var_dump($a);
}

function test(): void {
  echo "hello\n";
  echo 'hello\n';
  $x = "hello\n";
  $y = 'hello\n';
  $a = array();
  $a[$x] = 5;
  $a["hello\n"]++;
  $a[$y] = 50;
  $a['hello\n']++;
  var_dump($a);

  dump(Cns::X1);
  dump(Cns::X2);
  dump(Cns::X3);
  dump(Cns::X4);
  dump("\x00\x00\x00\x00");
  dump("\x70\x00\xff\x00");
  dump("\a\b\c\d\e\f\g\h\i\j\k\l\m\n\o\p\q\r\s\t\u\v\w\x\y\z");
  dump("\xa");
  dump("\xag\008");
  dump("\xag\123");

  var_dump("\nope \
  hello");

  var_dump('\\\'');

  dump("\u{07}\u{7F}\u{7E}\u{80}\u{AE}\u{3AA}\u{8000}\u{8c73}");
  dump("\u{10000}\u{FCA97}\u{10FFFE}");

  // Make sure we properly escape the \s in namespace things
  // (HH\ will be implicitly added here)
  var_dump(new Vector());
}
