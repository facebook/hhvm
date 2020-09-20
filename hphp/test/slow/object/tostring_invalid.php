<?hh

class ReturnsString {
  public function __toString() {
    return self::class;
  }
}

class ReturnsThis {
  public function __toString() {
    return $this;
  }
}

class ReturnsInt {
  public function __toString() {
    return 42;
  }
}

class ReturnsNull {
  public function __toString() {
    return null;
  }
}

class ReturnsStringable {
  public function __toString() {
    return new ReturnsString();
  }
}


<<__EntryPoint>>
function main_tostring_invalid() {
set_error_handler(
  function($errno, $str) {
    var_dump($str);
  }
);

var_dump((string) (new ReturnsString()));

var_dump((string) (new ReturnsThis()));

var_dump((string) (new ReturnsInt()));

var_dump((string) (new ReturnsInt()));

var_dump((string) (new ReturnsStringable()));
}
