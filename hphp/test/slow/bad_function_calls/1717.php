<?hh


<<__EntryPoint>>
function main_1717() :mixed{
$vals = vec[null, 0, false, vec[], 'test'];
foreach ($vals as $val) {
  try {
    $val->foo();
  }
 catch (BadMethodCallException $e) {
    echo "BadMethodCallException thrown\n";
  }
}
}
