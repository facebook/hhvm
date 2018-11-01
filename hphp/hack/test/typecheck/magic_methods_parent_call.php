<?hh // partial

class Example {
  public function __toString(): string {
    return 'parent test';
  }

  public function __call(string $name, string $args): string {
    return 'invoke parent';
  }
}

class SecondExample extends Example {
  public function __call(string $name, string $args): string {
    parent::__call('one', 'two');
    return 'test';
  }
}


function test():void {
  $t = new Example();
  $t->__toString();
}
