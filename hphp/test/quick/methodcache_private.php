<?hh

class one {
  public function lol() :mixed{}

  public function doit($o) :mixed{
    $o->heh();
  }

  private function heh() :mixed{
    echo "one\n";
  }
}

class two extends one {
  private function heh() :mixed{
    echo "two\n";
  }
}

<<__EntryPoint>> function main(): void {
  $one = new one;
  $two = new two;
  $one->doit($two);
  $one->doit($one);
  $one->doit($one);
  $one->doit($two);
}
