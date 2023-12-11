<?hh

abstract final class ForeachLoopIteratorAggregate {
  public static $indent;
}

class EnglishMealIterator implements Iterator {
    private $pos=0;
    private $myContent=vec["breakfast", "dinner", "tea"];

    public function valid() :mixed{
        $indent = ForeachLoopIteratorAggregate::$indent;
        $indent__str = (string)($indent);
        echo "$indent__str--> " . __METHOD__ . " ($this->pos)\n";
        return $this->pos < count($this->myContent);
    }

    public function next() :mixed{
        $indent = ForeachLoopIteratorAggregate::$indent;
        $indent__str = (string)($indent);
        echo "$indent__str--> " . __METHOD__ . " ($this->pos)\n";
        $this->pos++;
    }

    public function rewind() :mixed{
        $indent = ForeachLoopIteratorAggregate::$indent;
        $indent__str = (string)($indent);
        echo "$indent__str--> " . __METHOD__ . " ($this->pos)\n";
        $this->pos=0;
    }

    public function current() :mixed{
        $indent = ForeachLoopIteratorAggregate::$indent;
        $indent__str = (string)($indent);
        echo "$indent__str--> " . __METHOD__ . " ($this->pos)\n";
        return $this->myContent[$this->pos];
    }

    public function key() :mixed{
        $indent = ForeachLoopIteratorAggregate::$indent;
        $indent__str = (string)($indent);
        echo "$indent__str--> " . __METHOD__ . " ($this->pos)\n";
        return "meal " . $this->pos;
    }

}

class FrenchMealIterator implements Iterator {
    private $pos=0;
    private $myContent=vec["petit dejeuner", "dejeuner", "gouter", "dinner"];

    public function valid() :mixed{
        $indent = ForeachLoopIteratorAggregate::$indent;
        $indent__str = (string)($indent);
        echo "$indent__str--> " . __METHOD__ . " ($this->pos)\n";
        return $this->pos < count($this->myContent);
    }

    public function next() :mixed{
        $indent = ForeachLoopIteratorAggregate::$indent;
        $indent__str = (string)($indent);
        echo "$indent__str--> " . __METHOD__ . " ($this->pos)\n";
        $this->pos++;
    }

    public function rewind() :mixed{
        $indent = ForeachLoopIteratorAggregate::$indent;
        $indent__str = (string)($indent);
        echo "$indent__str--> " . __METHOD__ . " ($this->pos)\n";
        $this->pos=0;
    }

    public function current() :mixed{
        $indent = ForeachLoopIteratorAggregate::$indent;
        $indent__str = (string)($indent);
        echo "$indent__str--> " . __METHOD__ . " ($this->pos)\n";
        return $this->myContent[$this->pos];
    }

    public function key() :mixed{
        $indent = ForeachLoopIteratorAggregate::$indent;
        $indent__str = (string)($indent);
        echo "$indent__str--> " . __METHOD__ . " ($this->pos)\n";
        return "meal " . $this->pos;
    }

}


class EuropeanMeals implements IteratorAggregate {

    private $storedEnglishMealIterator;
    private $storedFrenchMealIterator;

    public function __construct() {
        $this->storedEnglishMealIterator = new EnglishMealIterator;
        $this->storedFrenchMealIterator = new FrenchMealIterator;
    }

  private static $getIteratorI = 0;

    public function getIterator() :mixed{
        $indent = ForeachLoopIteratorAggregate::$indent;
        $indent__str = (string)($indent);
        echo "$indent__str--> " . __METHOD__  . "\n";
        if (self::$getIteratorI++%2 == 0) {
            return $this->storedEnglishMealIterator;
        } else {
            return $this->storedFrenchMealIterator;
        }
    }

}
<<__EntryPoint>> function main(): void {
$f = new EuropeanMeals;
var_dump($f);

echo "-----( Simple iteration 1: )-----\n";
foreach ($f as $k=>$v) {
    echo "$k => $v\n";
}
echo "-----( Simple iteration 2: )-----\n";
foreach ($f as $k=>$v) {
    echo "$k => $v\n";
}


ForeachLoopIteratorAggregate::$indent = " ";
echo "\n\n\n-----( Nested iteration: )-----\n";
$count=1;
foreach ($f as $k=>$v) {
    echo "\nTop level "  .  $count++ . ": \n";
    echo "$k => $v\n";
  ForeachLoopIteratorAggregate::$indent = "     ";
    foreach ($f as $k=>$v) {
        echo "     $k => $v\n";
    }
  ForeachLoopIteratorAggregate::$indent = " ";
}


echo "===DONE===\n";
}
