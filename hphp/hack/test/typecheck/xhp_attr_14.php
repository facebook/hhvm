<?hh

class :foo extends XHPTest {
  public function render(): void {
    var_dump($this->:data-undefined);
    var_dump($this->:aria-undefined);
    var_dump($this->:undefined);
  }
}
