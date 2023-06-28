<?hh

function add_cssclass($add, $class) :mixed{
  $class = (!($class ?? false)) ? $add : $class .= ' ' . $add;
  return $class;
}
<<__EntryPoint>> function main(): void {
  try {
    add_cssclass('test', $a);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
