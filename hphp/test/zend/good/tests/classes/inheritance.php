<?hh

/* Inheritance test.  Pretty nifty if I do say so myself! */

class foo {
  public $a;
  public $b;
  function display() :mixed{
  	echo "This is class foo\n";
    echo "a = ".$this->a."\n";
    echo "b = ".$this->b."\n";
  }
  function mul() :mixed{
    return $this->a*$this->b;
  }
}

class bar extends foo {
  public $c;
  function display() :mixed{  /* alternative display function for class bar */
    echo "This is class bar\n";
    echo "a = ".$this->a."\n";
    echo "b = ".$this->b."\n";
    echo "c = ".$this->c."\n";
  }
}

<<__EntryPoint>> function main(): void {
$foo1 = new foo;
$foo1->a = 2;
$foo1->b = 5;
$foo1->display();
echo $foo1->mul()."\n";

echo "-----\n";

$bar1 = new bar;
$bar1->a = 4;
$bar1->b = 3;
$bar1->c = 12;
$bar1->display();
echo $bar1->mul()."\n";
}
