<?hh
function handler($e) {
  var_dump(strpos((string)$e, 'bomb') !== false);
  return true;
}
function a() {
}
<<__EntryPoint>> function main(): void {
  set_exception_handler(fun('handler'));
  set_exception_handler(fun('a'));
  restore_exception_handler();
  throw new Exception('bomb');
}
