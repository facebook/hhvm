<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

interface I {
  abstract const X = 1;
}

<<__EntryPoint>>
function main(): void {
  echo "ok";
}
