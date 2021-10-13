<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

// Make sure that enum class constants are allowed to be used as labels.
// Right now they are both 'self.require_name_allow_all_keywords'

enum class E : int {
  int list = 42;
}

enum class F : int {
  int List = 42;
}

enum class G : int {
  int LIST = 42;
}

function getE(HH\EnumClass\Label<E, int> $x): int {
  return E::valueOf($x);
}

function getF(HH\EnumClass\Label<F, int> $x): int {
  return F::valueOf($x);
}

function getG(HH\EnumClass\Label<G, int> $x): int {
  return G::valueOf($x);
}

<<__EntryPoint>>
function main(): void {
  echo getE(#list);
  echo "\n";
  echo getF(#List);
  echo "\n";
  echo getG(#LIST);
  echo "\n";
}
