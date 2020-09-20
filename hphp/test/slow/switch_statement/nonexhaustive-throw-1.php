<?hh

function f($x) {
  switch($x) {
    case 1:
      return 1;
    case 2:
  }
  return 2;
}

<<__EntryPoint>>
function main() {
  try { var_dump(f(1)); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump(f(2)); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump(f(3)); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump(f(4)); } catch (Exception $e) { var_dump($e->getMessage()); }
}
