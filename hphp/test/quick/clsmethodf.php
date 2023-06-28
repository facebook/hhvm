<?hh

class aa {
  protected function blah() :mixed{
    echo "protected aa::blah\n";
    var_dump($this);
  }

  protected function func($o) :mixed{
    echo "protected aa::blah\n";
    var_dump($o === $this);
  }
}

class a extends aa {
  protected function blah() :mixed{
    echo "private a::blah\n";
  }

  public static function stat() :mixed{
    echo "public static a::stat\n";
  }

  public function nons() :mixed{
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
