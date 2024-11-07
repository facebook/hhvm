////file1.php
<?hh

<<file:__EnableUnstableFeatures('class_type')>>

class C {}
function ret(): class<C> { return C::class; }

////file2.php
<?hh

function main(): void {
  hh_show(ret());
}
