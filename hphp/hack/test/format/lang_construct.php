<?hh

class A {

  public function test() {
    if (empty($aReallyLongVariableNameThatJustKeepsOnGoingUntilItForcesALineWrap)) {
      return $this;
    }
    if (isset($aReallyLongVariableNameThatJustKeepsOnGoingUntilItForcesALineWrap)) {
      return $this;
    }
    unset($aReallyLongVariableNameThatJustKeepsOnGoingUntilItForcesALineWrapForLonger);
  }
}
