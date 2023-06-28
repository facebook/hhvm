<?hh


<<__EntryPoint>>
function main_1717() :mixed{
$vals = varray[null, 0, false, varray[], 'test'];
foreach ($vals as $val) {
  try {
    $val->foo();
  }
 catch (BadMethodCallException $e) {
    echo "BadMethodCallException thrown\n";
  }
}
}
