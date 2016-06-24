<?hh
function f() {
  foreach ($arr as $v){
  }
  foreach ($arr as $k => $v){
    $k++;
  }
  foreach ($arr await as $v){
  }
  foreach ($arr await as $k => $v){
  }
}
