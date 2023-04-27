//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

new module here {}
new module there {}
new module elsewhere {}

//// here.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
module here;

function foo():void { }

type Talias = int;

newtype Topaque = string;

//// there.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
module there;

class C {
  public function bar():void { }
}

//// elsewhere.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
module elsewhere;


enum E : int {
  A = 3;
}
