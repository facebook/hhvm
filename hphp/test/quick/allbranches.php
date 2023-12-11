<?hh

trait T {
  public function binary($la, $ra) :mixed{
    // Stick this in a trait, so we get different copies for different
    // data.
    for ($i = 0; $i < 3; $i++) {
      // Compare each to each. Do so a couple times, so we exercise both
      // directions of each branch through multiple executions. (The jit
      // uses different code for the first several executions of any given
      // branch.)
      foreach ($la as $l) foreach ($ra as $r) {
        if ($l === $r) echo (string)$l." === ".(string)$r."\n";
        if ($l !== $r) echo (string)$l." !== ".(string)$r."\n";
        if ($l == $r) echo (string)$l." == ".(string)$r."\n";
        if ($l != $r) echo (string)$l." != ".(string)$r."\n";
        if ($l <= $r) echo (string)$l." <= ".(string)$r."\n";
        if ($l >= $r) echo (string)$l." >= ".(string)$r."\n";
        if ($l < $r) echo (string)$l." < ".(string)$r."\n";
        if ($l > $r) echo (string)$l." > ".(string)$r."\n";
      }
    }
  }
  public function unary($la) :mixed{
    for ($i = 0; $i < 3; $i++) {
      foreach($la as $l) {
        $text = HH\is_any_array($l) ? 'Array' : $l;
        if (is_int($l)) echo (string)$text." is_int\n";
        if (is_string($l)) echo (string)$text." is_string\n";
        if (is_double($l)) echo (string)$text." is_double\n";
        if (is_null($l)) echo (string)$text." is_null\n";
        if (is_double($l)) echo (string)$text." is_double\n";
        if (HH\is_any_array($l)) echo (string)$text." is_array\n";
        if (is_object($l)) echo (string)$text." is_object\n";
      }
    }
  }
}

function banner($s) :mixed{
  printf("---- %40s\n", $s);
}

class Ascending {
  use T;
  function __construct() {
    banner("asc");
    $this->binary(vec[-1, 0, 1], vec[-1, 0, 1]);
  }
}

class Descending {
  use T;
  function __construct() {
    banner("desc");
    $this->binary(vec[1, 0, -1], vec[1, 0, -1]);
  }
}

class Equal {
  use T;
  function __construct() {
    banner("eq");
    $this->binary(vec[0, 0, 0], vec[0, 0, 0]);
  }
}

class Str {
  use T;
  function __construct() {
    $a = vec["a", "abc", "abcd", "0", "1", "2"];
    $this->binary($a, $a);
  }
}
class NotEqual {
  use T;
  function __construct() {
    banner("neq");
    $this->binary(vec[1, 2, 3], vec[4, 5, 6]);
  }
}

class Bools {
  use T;
  function __construct() {
    banner("bools");
    $this->binary(vec[false, true], vec[false, true]);
  }
}

class C {
  public function __toString() :mixed{ return "snee!"; }
}

class DifferentTypes {
  use T;
  function __construct() {
    banner("weirdTypes");
    $a = vec[0, true, null, 0.3, "str", vec[], new C()];
    $this->unary($a);
  }
}

<<__EntryPoint>>
function main() :mixed{
  $asc = new Ascending();
  $desc = new Descending();
  $eq = new Equal();
  $str = new Str();
  $neq = new NotEqual();
  $bools = new Bools();
  $diff = new DifferentTypes();
}
