<?hh
<<__EntryPoint>>
function main(): void {
  try {
    $data = $not_exists;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $data = ($not_exists);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $data = !$not_exists;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $data = !$not_exists;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $data = ($not_exists + 1);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  echo "ok\n";
}
