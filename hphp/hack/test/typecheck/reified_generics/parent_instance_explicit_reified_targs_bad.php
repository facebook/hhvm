<?hh

class ReifiedParent {
  public function passThrough<reify T>(T $value): T {
    return $value;
  }
}

final class ReifiedChild extends ReifiedParent {
  public function callParent<reify T>(T $value): T {
    return parent::passThrough<T>($value);
  }
}
