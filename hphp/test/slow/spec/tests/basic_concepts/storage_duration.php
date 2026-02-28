<?hh

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
:mixed  {
    $this->x = $x;
    $this->y = $y;
  }

  public function translate($x, $y)
:mixed  {
    $this->x += $x;
    $this->y += $y;
  }

  public function __toString()
:mixed  {
    return '(' . $this->x . ',' . $this->y . ')';
  }
}

class State { public static $sv1 = TRUE; public static $sv2 = 0; public static $sv3 = NULL; }

function doit($p1)
:mixed{
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

function factorial($i)
:mixed{
  if ($i > 1) return $i * factorial($i - 1);
  else if ($i == 1) return $i;
  else return 0;
}
<<__EntryPoint>>
function entrypoint_storage_duration(): void {

  /*
     +-------------------------------------------------------------+
     | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
     +-------------------------------------------------------------+
  */

  error_reporting(-1);

  echo "---------------- start -------------------\n";

  $av1 = new Point(0, 1);

  echo "---------------- after \$av1 init -------------------\n";

  echo "---------------- after \$sv1 decl -------------------\n";

  State::$sv1 = new Point(0, 2);

  echo "---------------- after \$sv1 init -------------------\n";

  doit(TRUE);

  echo "---------------- after call to func -------------------\n";

  $count = 10;
  $result = factorial($count);
  echo "\$result = $result\n";

  echo "---------------- end -------------------\n";
}
