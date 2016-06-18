<?hh
function f(){
  try {
    $a++;
  }
  catch (Bar $a){
    $b;
  }
  finally {
    $c;
  }
}
