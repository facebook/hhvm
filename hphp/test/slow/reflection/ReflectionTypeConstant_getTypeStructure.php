<?hh

abstract class MyFooAbstract {
  abstract const type TAbstract;
  const type TConcrete = this::TAbstract;
}

final class MyFoo extends MyFooAbstract {
  const type TAbstract = int;
}

<<__EntryPoint>>
function main(): void {
  $tc = new ReflectionTypeConstant(MyFoo::class, 'TConcrete');
  var_dump($tc->getTypeStructure());
}
