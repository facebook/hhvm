<?hh
function f(Bar $vector){
  list($a, $b) = $vector;
  list($a, list($b, $c)) = $vector;
  list(,,$a,,) = $vector; // Missing binders should be `$_`-style ignores.
}
