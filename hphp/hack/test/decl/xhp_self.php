<?hh

type REACT_XHP_FIX<+T> = T;

final class :Foo extends XHPTest {
  const type TProps = shape("x" => int);

  attribute REACT_XHP_FIX<self::TProps> props @required;

  protected function getProps(): self::TProps {
    return $this->:props;
  }
}
