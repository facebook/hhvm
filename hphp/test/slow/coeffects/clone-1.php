<?hh

class C {
  public int $p;
  function __clone()[] :mixed{ $this->p = 2; }
  function __construct()[] { $this->p = 1; }
}

<<__EntryPoint>>
function main()[] :mixed{
  $c = new C();
  echo $c->p;
  $c2 = clone $c;
  echo $c2->p;
  echo "done\n";
}
