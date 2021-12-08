<?hh

<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

class A {
  <<__Internal>>
  private function bad1(): void {}
  <<__Internal>>
  protected function bad2(): void {}
  <<__Internal>>
  public function good(): void {}
}
