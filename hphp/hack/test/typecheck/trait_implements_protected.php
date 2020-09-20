<?hh // strict

interface I {
  protected function getProtected(): void;
}

trait T implements I {
  public function f(): void {
    $this->getProtected();
  }
}
