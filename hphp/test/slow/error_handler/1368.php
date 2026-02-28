<?hh
function handler($e) :mixed{
  var_dump(strpos((string)$e, 'bomb') !== false);
  return true;
}
function a() :mixed{
}
<<__EntryPoint>> function main(): void {
  set_exception_handler(handler<>);
  set_exception_handler(a<>);
  restore_exception_handler();
  throw new Exception('bomb');
}
