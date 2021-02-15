<?hh

class :hello extends XHPTest {}
class :selam extends XHPTest implements XHPChild {}
class :salut extends XHPTest implements XHPChild {}

function f(): void {
  <hello>
    <selam />
    <salut />
  </hello>;
}
