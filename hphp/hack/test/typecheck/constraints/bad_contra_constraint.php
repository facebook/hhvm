<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Base<-Tb> {}
class Derived<Td> extends Base<Td> {
  public function __construct(
    private ConstVector<Pair<Base<Td>, float>> $item,
  ) {}
  public static function getStuff(ConstVector<mixed> $v): Derived<Td> {
    $args = Vector {};
    $i = 0;
    $el = $v[$i];
    if (!$el is Base<_>) {
      throw new Exception('a');
    }
    // So here, $el has type Base<Tb#1>
    // So $args has type Vector<Pair<Base<Tb#1>,float>>
    $args->add(Pair { $el, 3.4 });
    // We need Base<Tb#1> <: Base<Td>
    // So by contravariance we require Td <: Tb#1
    // Why would these be related??
    return new self($args);
  }
}
