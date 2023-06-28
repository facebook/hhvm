<?hh

class one {
  public function doit($o) :mixed{
    // Method cache should still dispatch to one::heh for $o :: two
    $o->heh();
  }

  private function heh() :mixed{
    echo "one\n";
  }
}

class two extends one {
  // You can override a private function with a public one in php.
  // (But you can't change it to static.)
  public function heh() :mixed{
    echo "two\n";
  }
}

<<__EntryPoint>> function main(): void {
  $one = new one;
  $two = new two;
  $one->doit($one);
  $one->doit($two);
  $one->doit($two);
  $one->doit($one);
  $one->doit($one);
  $one->doit($two);
  $one->doit($two);
}
