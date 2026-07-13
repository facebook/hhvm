<?hh

function add_cssclass($add, $class) :mixed{
  if (!($class ?? false)) { $class = $add; } else { $class .= ' ' . $add; }
  return $class;
}
<<__EntryPoint>> function main(): void {
  try {
    add_cssclass('test', $a);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
