<?hh // strict

$x = null;
$foo = 'foo';
if (false) {
  $x?->${foo}; // parse error
}
