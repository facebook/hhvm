//// f.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newtype X as [] = [HH\Contexts\defaults];

function internal()[\X]: void {
  default_lib_func(); // ok
}

function default_lib_func(): void {}

//// g.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

function external()[\X]: void {
  default_lib_func(); // error, X is not <: defaults
  pure_ok(); // ok
}

function pure_ok()[]: void {}
