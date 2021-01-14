<?hh
class aclass {
  const myConst = "hello";
}
<<__EntryPoint>>
function main(): void {
  echo "\nRead class constant.\n";
  var_dump(aclass::myConst);

  echo "\nFail to read class constant from instance.\n";
  $myInstance = new aclass();
  try {
    var_dump($myInstance->myConst);
  } catch (UndefinedPropertyException $e) {
    var_dump($e->getMessage());
  }

  echo "\nClass constant not visible in object var_dump.\n";
  var_dump($myInstance);
}
