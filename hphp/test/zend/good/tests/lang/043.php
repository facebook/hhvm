<?hh

class A {
  static function foo() :mixed{ return 'foo'; }
}

<<__EntryPoint>>
function main(): void {
  $classname       =  'A';
  $wrongClassname  =  'B';

  echo $classname::foo()."\n";
  echo $wrongClassname::foo()."\n";
  echo "===DONE===\n";
}
