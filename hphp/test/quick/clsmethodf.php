<?hh

class aa {
  protected function blah() {
    echo "protected aa::blah\n";
    var_dump($this);
  }

  protected function func($o) {
    echo "protected aa::blah\n";
    var_dump($o === $this);
  }
}

class a extends aa {
  protected function blah() {
    echo "private a::blah\n";
  }

  public static function stat() {
    echo "public static a::stat\n";
  }

  public function nons() {
    $str = 'blah';
    self::$str();
    self::stat();
    parent::blah();

    parent::func(null);
    parent::func($this);
    parent::func(null);
    parent::func($this);
  }
}

<<__EntryPoint>> function main(): void {
  $a = new a();
  $a->nons();
}
