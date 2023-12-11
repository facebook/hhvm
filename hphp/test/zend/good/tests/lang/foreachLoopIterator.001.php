<?hh

class MealIterator implements Iterator {
  public static $indent;
    private $pos=0;
    private $myContent=vec["breakfast", "lunch", "dinner"];

    public function valid() :mixed{
        echo (string)(self::$indent)."--> " . __METHOD__ . " ($this->pos)\n";
        return $this->pos<3;
    }

    public function next() :mixed{
      echo (string)(self::$indent)."--> " . __METHOD__ . " ($this->pos)\n";
      try {
        return $this->myContent[$this->pos++];
      } catch (Exception $e) { echo $e->getMessage()."\n"; }
    }

    public function rewind() :mixed{

        echo (string)(self::$indent)."--> " . __METHOD__ . " ($this->pos)\n";
        $this->pos=0;
    }

    public function current() :mixed{

        echo (string)(self::$indent)."--> " . __METHOD__ . " ($this->pos)\n";
        return $this->myContent[$this->pos];
    }

    public function key() :mixed{

        echo (string)(self::$indent)."--> " . __METHOD__ . " ($this->pos)\n";
        return "meal " . $this->pos;
    }

}
<<__EntryPoint>> function main(): void {
$f = new MealIterator;
var_dump($f);

echo "-----( Simple iteration: )-----\n";
foreach ($f as $k=>$v) {
    echo "$k => $v\n";
}

$f->rewind();

MealIterator::$indent = " ";

echo "\n\n\n-----( Nested iteration: )-----\n";
$count=1;
foreach ($f as $k=>$v) {
    echo "\nTop level "  .  $count++ . ":\n";
    echo "$k => $v\n";
  MealIterator::$indent = "     ";
    foreach ($f as $k=>$v) {
        echo "     $k => $v\n";
    }
  MealIterator::$indent = " ";

}

echo "===DONE===\n";
}
