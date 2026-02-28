<?hh

class FluentObj {
  private
    $first,
    $second,
    $third,
    $fourth,
    $fifth;

  public function __construct($first) {
    $this->first = $first;
  }

  public function setSecond($x) :mixed{
    $this->second = $x;
    return $this;
  }
  public function setThird($x) :mixed{
    $this->third = $x;
    return $this;
  }
  public function setFourth($x) :mixed{
    $this->fourth = $x;
    return $this;
  }
  public function setFifth($x) :mixed{
    $this->fifth = $x;
    return $this;
  }

  public function getValue() :mixed{
    return $this->first;
  }
}

function main() :mixed{
  $k = (new FluentObj('a'))
    ->setSecond('b')
    ->setThird('c')
    ->setFourth('d')
    ->setFifth('e')
    ->getValue();
  echo $k;
  echo "\n";
}

<<__EntryPoint>>
function main_fluent() :mixed{
main();
}
