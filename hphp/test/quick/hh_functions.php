<?hh
function compose<X,Y,Z>((function(Y):Z) $f, (function(X):Y) $g):(function(X):Z) {
  return function(X $x):Z use($f, $g) {
    return $f($g($x));
  };
}
<<__EntryPoint>> function main(): void {
$x = compose('htmlspecialchars_decode', 'htmlspecialchars');

for($i=0;$i<256;$i++) {
  if (chr($i) !== $x(chr($i))) {
    echo "[$i]\n";
  }
}
}
