<?hh

class BaseDerp {
  public function genDerp() :AsyncGenerator<mixed,mixed,void>{
    yield 'derp';
  }
}

class ShortDerp extends BaseDerp {}
<<__EntryPoint>> function main(): void {
$sd = new ShortDerp;
var_dump($sd->genDerp()->getOrigFuncName());
var_dump($sd->genDerp()->getCalledClass());
}
