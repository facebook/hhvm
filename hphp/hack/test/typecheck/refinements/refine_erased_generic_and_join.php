<?hh

interface IPolicy<-T> {
  public function getRule(): IRule<T>;
}

interface IRule<-T> {
  public function getSubPolicy(): IPolicy<T>;
}

interface DenyRule<T> extends IRule<T> {}
interface AllowRule<T> extends IRule<T> {}

final class MyClass<T> {
  private function getAllRulesRec(IPolicy<T> $policy): vec<IRule<T>> {
    $rule = $policy->getRule();
    if ($rule is DenyRule<_>) {
      ;
    } else {
      if ($rule is AllowRule<_>) {
        ;
      } else {
        return vec[];
      }
    }
    $sp = $rule->getSubPolicy();
    return $this->getAllRulesRec($sp);
  }
}
