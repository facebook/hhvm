<?hh
xhp class div extends XHPTest {}

xhp class test:default_attributes extends XHPTest {
  attribute string mystring = 'mydefault';

  protected function render(): :div {
    return <div>{$this->:mystring}</div>;
  }

  protected function getStr(): string {
    return $this->:mystring;
 }
}

xhp class test:parent_class extends XHPTest {
  attribute string parentstring @required;
}

xhp class test:child_class extends XHPTest {
  attribute
    :test:parent_class,
    string childstring @required;

  public function render(): div {
    return <div>{$this->:parentstring}{$this->:childstring}</div>;
  }
}
