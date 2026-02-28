<?hh


function go($c, $k) :mixed{
  try {
    $unused = $c[$k];
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    $c[$k] = 0;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  var_dump($c);
}
<<__EntryPoint>> function main(): void {
go(Vector {'zero', 'one'}, 2);
go(Vector {'zero', 'one'}, -2);
go(Pair {'zero', 'one'}, 2);
}
