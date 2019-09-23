<?hh
class Ref{
  public function __construct(private mixed $value){}
}

    $a = new Ref('a');
    $b = new Ref('b');
    $c = new Ref('c');

    $foo = new SplObjectStorage;
    $foo->attach($a);
    $foo->attach($b);

    $bar = new SplObjectStorage;
    $bar->attach($b);
    $bar->attach($c);

    $foo->removeAllExcept($bar);
    var_dump($foo->contains($a));
    var_dump($foo->contains($b));

