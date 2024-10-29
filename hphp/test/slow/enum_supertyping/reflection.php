<?hh

enum Enum1: string as string {
  A = 'A';
}

enum Enum2: string as string {
  B = 'B';
}

enum Enum3: string as string {
  C = 'C';
}

enum CombinedEnums: string as string {
  use Enum1, Enum2, Enum3;
  D = 'D';
}


enum class Base: int {
  int BA = 1;
}

enum class Extd : int extends Base {
  int BB = 2;
}

<<__EntryPoint>>
function main(): void {
  $reflector = new ReflectionClass(Enum1::class);
  var_dump($reflector->getConstants());
  $reflector = new ReflectionClass(CombinedEnums::class);
  var_dump($reflector->getConstants());

  $reflector = new ReflectionClass(Base::class);
  var_dump($reflector->getConstants());

  $reflector = new ReflectionClass(Extd::class);
  var_dump($reflector->getConstants());
}
