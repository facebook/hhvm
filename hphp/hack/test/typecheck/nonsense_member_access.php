<?hh

class Foo {
  public function normalCases(): void {
    $this->$this;
    $this->$this->baz();
    $this->$this();
    $this->$_;
    $this->$_->baz();
    $this->$_();
  }
  public function onDynamic(dynamic $d): void {
    $d->$this;
    $d->$this->baz();
    $d->$this();
    $d->$_;
    $d->$_->baz();
    $d->$_();
  }
}
