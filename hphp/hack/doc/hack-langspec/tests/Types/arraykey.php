<?hh // strict

namespace NS_arraykey;

class C {
  const arraykey KEY = 100;
  private arraykey $prop = 'zzz';

  public function setProp(arraykey $val): void {
    $this->prop = $val;
  }

  public function getProp(): arraykey {
    return $this->prop;
  }

  public function __construct() {
    echo "arraykey " . $this->prop . (is_int($this->prop) ? " is int\n" : " is string\n");

    $this->prop = 20;
    echo "arraykey " . $this->prop . (is_int($this->prop) ? " is int\n" : " is string\n");

    $this->prop = 'red';
    echo "arraykey " . $this->prop . (is_int($this->prop) ? " is int\n" : " is string\n");
  }
}

function main (): void {
  $c = new C();
  $val = $c->getProp();

//  $q = $val + 10;	// checker can't be sure teh arraykey is currently an int

  if (is_int($val)) {
    $q = $val + 10;
  } else {
// Checker reports: This is not a container, this is an array key (int/string)
// It appears to not be able to deduce the type must be string; however, the true of the is_string (below) works.
//  echo "First character of string is " . $val[0] . "\n";
  }

  if (is_string($val)) {
    echo "First character of string is " . $val[0] . "\n";
  }
}

/* HH_FIXME[1002] call to main in strict*/
main();
