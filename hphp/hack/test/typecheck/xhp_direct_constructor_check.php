<?hh

class :c {
  public function __construct(int $i) {}
}

function f(): void {
  new :c('Hi');
}
