<?hh
<<file:__EnableUnstableFeatures('abstract_enum_class')>>

abstract enum class E : mixed {
  abstract int A;
}

<<__EntryPoint>>
function main(): void {
  echo "attribute 'abstract_enum_class' is still valid syntax";
}
