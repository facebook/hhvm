<?hh

class ReturnsString {
  public function __toString()[] :mixed{
    return self::class;
  }
}

class ReturnsThis {
  public function __toString() :mixed{
    return $this;
  }
}

class ReturnsInt {
  public function __toString()[] :mixed{
    return 42;
  }
}

class ReturnsNull {
  public function __toString()[] :mixed{
    return null;
  }
}

class ReturnsStringable {
  public function __toString() :mixed{
    return new ReturnsString();
  }
}


<<__EntryPoint>>
function main_tostring_invalid() :mixed{
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
