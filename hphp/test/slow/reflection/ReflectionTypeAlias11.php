<?hh

<<__EntryPoint>> function main(): void {
  require_once 'ReflectionTypeAlias.inc';
  require_once 'ReflectionTypeAlias11.inc';
  $x = new ReflectionTypeAlias('MyType');
  var_dump($x->getFileName());

  $x = new ReflectionTypeAlias('MyOpaqueType');
  var_dump($x->getFileName());
}
