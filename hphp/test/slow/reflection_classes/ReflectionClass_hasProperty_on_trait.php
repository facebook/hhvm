<?hh

interface IMyInterface {
  const int DEFAULT_INT = 1;
}

trait MyTrait {
  require implements IMyInterface;
  public int $withDefaultFromRequireImplements = self::DEFAULT_INT;
}

<<__EntryPoint>>
function mymain(): void {
  $cls = new ReflectionClass(nameof MyTrait);
  var_dump($cls->hasProperty('blahblah'));
  var_dump($cls->hasProperty('withDefaultFromRequireImplements'));
  $props = $cls->getProperties();
  var_dump($props);
}
