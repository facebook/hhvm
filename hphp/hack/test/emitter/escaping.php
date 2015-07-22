<?hh // strict

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

  // Make sure we properly escape the \s in namespace things *)
  /* HH_FIXME[2049] */
  /* HH_FIXME[4107] */
  var_dump(new HH\Vector());
}
