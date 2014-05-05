<?hh

class box {
  public function __construct(private $x) {}
  public function get() { return $this->x; }
}

function mappers() {
  $dump = ($ar, $fn) ==> {
    var_dump(array_map($fn, $ar));
  };

  // Doesn't capture $x; the parameter wins:
  $x = array("one", "two", "three");
  $dump($x, $x ==> "(".$x.")");

  $dump(array(1,2,3,4), $x ==> $x*$x);
  $dump(
    array(new box(123), new box(456)),
    $x ==> $x->get()
  );
}
mappers();

echo "---\n";

function filters() {
  $dump = ($ar,$fn) ==> {
    var_dump(array_filter($ar, $fn));
  };

  $dump(array(1,2,3,4,5,6),         $x ==> $x % 2 == 0);
  $dump(array("a", "b", "ac", "k"), $x ==> $x[0] == "a");
  $dump(array("asd", new box(123)), $x ==> is_string($x));
}
filters();

function collection() {
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
collection();
