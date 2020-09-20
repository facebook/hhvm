<?hh

<<__EntryPoint>> function main(): void {
  require_once 'ReflectionTypeAlias.inc';
  require_once 'ReflectionTypeAlias10.inc';

  $x = new ReflectionTypeAlias('MyType2');
  var_dump($x->getFileName());

  $x = new ReflectionTypeAlias('MyOpaqueType2');
  var_dump($x->getFileName());
}
