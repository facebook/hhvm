<?hh

class Point2
{
    public $x;
    public $y;

    public function __construct($x = 0, $y = 0)
    {
//      echo "Inside " . __METHOD__ . "\n";

        $this->x = $x;
        $this->y = $y;
    }

    public function __toString()
:mixed    {
        return '(' . $this->x . ',' . $this->y . ')';
    }
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);
}
