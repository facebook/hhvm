//// f.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newtype Foo as [] = [defaults];

function enter_foo(
  (function ()[Foo]: void) $f
)[\Foo]: void {
  echo "Hello I can secretly do IO";
  $f();
}

//// g.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

// client code
<<__EntryPoint>>
function main()[Foo]: void {
  enter_foo(
    () ==> {
      echo "break purity"; // illegal!
    }
  );
}
