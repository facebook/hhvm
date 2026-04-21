<?hh
<<file:__EnableUnstableFeatures('class_aliases_everywhere')>>

class Original {}

class Alias = Original;

function foo(): void {
  new Original();
  new Alias();
}
