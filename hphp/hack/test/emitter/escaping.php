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

  // Make sure we properly escape the \s in namespace things
  // (HH\ will be implicitly added here)
  var_dump(new Vector());
}
