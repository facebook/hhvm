//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module here {}
module there {}
module elsewhere {}

//// here.php
<?hh
<<file:__EnableUnstableFeatures('modules'), __Module('here')>>

function foo():void { }

type Talias = int;

newtype Topaque = string;

//// there.php
<?hh
<<file:__EnableUnstableFeatures('modules'), __Module('there')>>

class C {
  public function bar():void { }
}

//// elsewhere.php
<?hh
<<file:__EnableUnstableFeatures('modules'), __Module('elsewhere')>>

<<__Module('elsewhere')>>
enum E : int {
  A = 3;
}
