<?

// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);

trait T {
  public function binary($la, $ra) {
    // Stick this in a trait, so we get different copies for different
    // data.
    for ($i = 0; $i < 3; $i++) {
      // Compare each to each. Do so a couple times, so we exercise both
      // directions of each branch through multiple executions. (The jit
      // uses different code for the first several executions of any given
      // branch.)
      foreach ($la as $l) foreach ($ra as $r) {
        if ($l === $r) echo "$l === $r\n";
        if ($l !== $r) echo "$l !== $r\n";
        if ($l == $r) echo "$l == $r\n";
        if ($l != $r) echo "$l != $r\n";
        if ($l <= $r) echo "$l <= $r\n";
        if ($l >= $r) echo "$l >= $r\n";
        if ($l < $r) echo "$l < $r\n";
        if ($l > $r) echo "$l > $r\n";
      }
    }
  }
  public function unary($la) {
    for ($i = 0; $i < 3; $i++) {
      foreach($la as $l) {
        if (is_int($l)) echo "$l is_int\n";
        if (is_string($l)) echo "$l is_string\n";
        if (is_double($l)) echo "$l is_double\n";
        if (is_null($l)) echo "$l is_null\n";
        if (is_double($l)) echo "$l is_double\n";
        if (is_array($l)) echo "$l is_array\n";
        if (is_object($l)) echo "$l is_object\n";
      }
    }
  }
}

function banner($s) {
  printf("---- %40s\n", $s);
}

class Ascending {
  use T;
  function __construct() {
    banner("asc");
    $this->binary(array(-1, 0, 1), array(-1, 0, 1));
  }
}

class Descending {
  use T;
  function __construct() {
    banner("desc");
    $this->binary(array(1, 0, -1), array(1, 0, -1));
  }
}

class Equal {
  use T;
  function __construct() {
    banner("eq");
    $this->binary(array(0, 0, 0), array(0, 0, 0));
  }
}

class Str {
  use T;
  function __construct() {
    $a = array("a", "abc", "abcd", "0", "1", "2");
    $this->binary($a, $a);
  }
}
class NotEqual {
  use T;
  function __construct() {
    banner("neq");
    $this->binary(array(1, 2, 3), array(4, 5, 6));
  }
}

class Bools {
  use T;
  function __construct() {
    banner("bools");
    $this->binary(array(false, true), array(false, true));
  }
}

class C {
  public function __toString() { return "snee!"; }
}

class DifferentTypes {
  use T;
  function __construct() {
    banner("weirdTypes");
    $a = array(0, true, null, 0.3, "str", array(), new C());
    $this->unary($a);
  }
}

function main() {
  $asc = new Ascending();
  $desc = new Descending();
  $eq = new Equal();
  $str = new Str();
  $neq = new NotEqual();
  $bools = new Bools();
  $diff = new DifferentTypes();
}
main();
