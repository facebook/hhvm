<?hh

class :hello {}
class :selam implements XHPChild {}
class :salut implements XHPChild {}

function f(): void {
  <hello>
    <selam />
    <salut />
  </hello>;
}
