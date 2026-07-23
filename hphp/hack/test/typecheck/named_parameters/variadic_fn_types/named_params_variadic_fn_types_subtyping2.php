<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class TypedMockFunction<Tfun> {
  public function mockReturn<Tr>(Tr $v): this
  where
    Tfun super (readonly function(mixed..., named mixed...)[]: Tr) {
    return $this;
  }
}

function mockFunction<Tfun>(HH\FunctionRef<Tfun> $fr): TypedMockFunction<Tfun> {
  return new TypedMockFunction();
}

function test1(int $x, bool $b = false): string {
  return "A";
}

function test2(int $x, named int $y): string {
  return "A";
}

function test3(int $x, bool $b = false, named int $y): string {
  return "A";
}

function test4(int $x, named int $y, named bool $b = false): string {
  return "A";
}

<<__EntryPoint>>
function main(): void {
  $mf1 = mockFunction(test1<>);
  $mf1->mockReturn("B");
  $mf2 = mockFunction(test2<>);
  $mf2->mockReturn("C");
  $mf3 = mockFunction(test3<>);
  $mf3->mockReturn("C");
  $mf4 = mockFunction(test4<>);
  $mf4->mockReturn("C");
}
