<?hh

class box {
  public function __construct(private $x) {}
  public function get() :mixed{ return $this->x; }
}

function mappers() :mixed{
  $dump = ($ar, $fn) ==> {
    var_dump(array_map($fn, $ar));
  };

  // Doesn't capture $x; the parameter wins:
  $x = vec["one", "two", "three"];
  $dump($x, $x ==> "(".$x.")");

  $dump(vec[1,2,3,4], $x ==> $x*$x);
  $dump(
    vec[new box(123), new box(456)],
    $x ==> $x->get()
  );
}

function filters() :mixed{
  $dump = ($ar,$fn) ==> {
    var_dump(array_filter($ar, $fn));
  };

  $dump(vec[1,2,3,4,5,6],         $x ==> $x % 2 == 0);
  $dump(vec["a", "b", "ac", "k"], $x ==> $x[0] == "a");
  $dump(vec["asd", new box(123)], $x ==> is_string($x));
}

function collection() :mixed{
  $blah = Vector {
    new box(1),
    new box(2),
    new box("3"),
    new box("4"),
    new box("5"),
    new box(6),
    new box(7)
  };

  var_dump(
    $blah->map($x ==> $x->get())
         ->filter($x ==> !is_string($x))
         ->filter($x ==> $x % 2 == 0)
  );
}

<<__EntryPoint>> function main(): void {
mappers();

echo "---\n";

filters();

collection();
}
