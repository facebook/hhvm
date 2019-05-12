<?hh

<<__EntryPoint>> function a(): void {
  $a = static function() { var_dump(true); };
  $a->__invoke();
}
