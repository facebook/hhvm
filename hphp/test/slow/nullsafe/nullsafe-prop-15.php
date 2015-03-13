<?hh // strict

$x = null;
$foo = 'foo';
if (false) {
  $x?->${$foo . "bar"}; // parse error
}
