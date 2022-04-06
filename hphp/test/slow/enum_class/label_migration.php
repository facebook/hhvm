<?hh

// This attributes still exists but don't gates anything anymore.
// We'll remove it once no more experiments will refer to it
<<file:__EnableUnstableFeatures('enum_class_label')>>

<<__EntryPoint>>
function main(): void {
  echo "Hello\n";
}
