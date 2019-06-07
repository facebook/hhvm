<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class Point
{
  private $x;
  private $y;

  public function __construct($x = 0, $y = 0)
  {
    $this->x = $x;
    $this->y = $y;
  }

  public function move($x, $y)
  {
    $this->x = $x;
    $this->y = $y;
  }

  public function translate($x, $y)
  {
    $this->x += $x;
    $this->y += $y;
  }

  public function __toString()
  {
    return '(' . $this->x . ',' . $this->y . ')';
  }
}

echo "---------------- start -------------------\n";

$av1 = new Point(0, 1);

echo "---------------- after \$av1 init -------------------\n";

class State { static $sv1 = TRUE; static $sv2 = 0; static $sv3 = NULL; }

echo "---------------- after \$sv1 decl -------------------\n";

State::$sv1 = new Point(0, 2);

echo "---------------- after \$sv1 init -------------------\n";

function doit($p1)
{
  echo "---------------- Inside function_A -------------------\n";

  $av2 = new Point(1, 1);

  echo "---------------- after \$av2 init -------------------\n";

  echo "---------------- after \$sv2 decl -------------------\n";

  State::$sv2 = new Point(1, 2);

  echo "---------------- after \$sv2 init -------------------\n";

  if ($p1)
  {
    echo "---------------- Inside if TRUE -------------------\n";

    $av3 = new Point(2, 1);

    echo "---------------- after \$av3 init -------------------\n";

    echo "---------------- after \$sv3 decl -------------------\n";

    State::$sv3 = new Point(2, 2);

    echo "---------------- after \$sv3 init -------------------\n";
    // ...
  }

  $av1 = new Point(2, 3);

  // Order of destruction is implementation-defined
  echo "---------------- after \$av1 reinit -------------------\n";
}

doit(TRUE);

echo "---------------- after call to func -------------------\n";

function factorial($i)
{
  if ($i > 1) return $i * factorial($i - 1);
  else if ($i == 1) return $i;
  else return 0;
}

$count = 10;
$result = factorial($count);
echo "\$result = $result\n";

echo "---------------- end -------------------\n";
