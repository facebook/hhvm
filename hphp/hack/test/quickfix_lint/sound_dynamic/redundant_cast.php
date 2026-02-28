<?hh

class Box<T> {
  public function __construct(private T $val): void {}
  public function getVal(): T { return $this->val; }
}

<<__SupportDynamicType>>
function not_sure_if_redundant_cast(Box<float> $box): void {
  (float) $box->getVal();
}

function redundant_cast(Box<float> $box): void {
  (float) $box->getVal();
}
