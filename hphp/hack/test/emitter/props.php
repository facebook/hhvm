<?hh // strict

class Cns {
  const C = 5;
}

class C {
  public int $arithmetic = 100+100;
  public int $intvar = 100;
  public array<int> $arrayvar1 = array(100);
  public array<array<int>> $arrayvar2 = array(array(100), array(200));
  public array<int> $arrayvar3 = array();
  public string $strvar1 = 'hello\n';
  public string $strvar2 = "hello\n";
  public bool $boolvar1 = true;
  public bool $boolvar2 = false;
  public float $floatvar = 3.1415;
  public ?bool $nullvar = null;
  public Map<int, int> $mapvar = Map {};
  public int $constvar = Cns::C;
}

function test(): void {
  $x = new C();
  var_dump($x->arithmetic);
  var_dump($x->intvar);
  var_dump($x->arrayvar1);
  var_dump($x->arrayvar2);
  var_dump($x->strvar1);
  var_dump($x->strvar2);
  var_dump($x->boolvar1);
  var_dump($x->boolvar2);
  var_dump($x->floatvar);
  var_dump($x->nullvar);
  var_dump($x->mapvar);
  var_dump($x->constvar);
}
