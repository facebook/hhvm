<?hh
function f(Bar $vector){
  list($a, $b) = $vector;
  list($a, list($b, $c)) = $vector;
}
