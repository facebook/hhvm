<?hh

class Point
{
    private $x;
    private $y;
    const CON = 10;
    protected static $prots;
    protected $proti;
    public $pubi;

    public function __construct($x = 0, $y = 0)
    {
        $this->x = $x;
        $this->y = $y;
    }

    public function move($x, $y)
:mixed    {
        $this->x = $x;
        $this->y = $y;
    }

    public function translate($x, $y)
:mixed    {
        $this->x += $x;
        $this->y += $y;
    }

    public function __toString()
:mixed    {
        return '(' . $this->x . ',' . $this->y . ')';
    }
///*
    static public function __set_state(darray $properties)
:mixed    {
        echo "Inside " . __METHOD__ . "\n";
        var_dump($properties);

        $p = new Point;
        $p->x = $properties['x'];
        $p->y = $properties['y'];
        var_dump($p);
        return $p;
    }
//*/
}

class B
{
    private $bprop;

    public function __construct($p)
    {
        $this->bprop = $p;
    }

    static public function __set_state(darray $properties)
:mixed    {
        echo "Inside " . __METHOD__ . "\n";
        var_dump($properties);

        $b = new static($properties['bprop']);
//      $b->bprop = $properties['bprop'];
        var_dump($b);
        echo "about to return from " . __METHOD__ . "\n";
        return $b;
    }
}

class D extends B
{
    private $dprop = 123;

    public function __construct($bp, $dp = NULL)
    {
        $this->dprop = $dp;
        parent::__construct($bp);
    }
///*
    static public function __set_state(darray $properties)
:mixed    {
        echo "Inside " . __METHOD__ . "\n";
        var_dump($properties);

        $d = parent::__set_state($properties);
        var_dump($d);
        $d->dprop = $properties['dprop'];
        var_dump($d);
        echo "about to return from " . __METHOD__ . "\n";
        return $d;
    }
//*/
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  echo "---------------- start -------------------\n";

  $p = new Point(3, 5);

  echo "---------------- calling var_export -------------------\n";

  $v = var_export($p, TRUE);
  var_dump($v);

  echo "---------------- calling eval -------------------\n";

  eval('function eval_point() { return ' . $v . ';}');
  $z = eval_point();
  echo "Point \$z is $z\n";

  unset($p, $v, $z);

  echo "---------------- test with inheritance -------------------\n";

  echo "---------------- test with type B -------------------\n";

  $b = new B(10);
  $v = var_export($b, TRUE);
  var_dump($v);

  eval('function eval_b() { return ' . $v . ';}');
  $z = eval_b();
  var_dump($z);

  echo "---------------- test with type D -------------------\n";

  $d = new D(20, 30);
  $v = var_export($d, TRUE);
  var_dump($v);

  eval('function eval_d() { return ' . $v . ';}');
  $z = eval_d();
  var_dump($z);

  echo "---------------- end -------------------\n";
}
