<?hh

class ParamBase {
}

class ParamSub extends ParamBase {
}

abstract class ImplBase {
  abstract public function doStuff(ParamSub $_): void;
}

class ImplSub extends ImplBase {
  public function doStuff(ParamBase $_): void {
  }
}

function main(): void {
  (new ImplSub())->doStuff(new ParamSub());
}


<<__EntryPoint>>
function main_abstract_variance() :mixed{
main();
}
