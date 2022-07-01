//// modules.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

new module cookies {}

//// test.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
module cookies;


internal class A {}

class B extends A{}
