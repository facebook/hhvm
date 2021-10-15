//// fileA.php
<?hh

<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>

newctx ABC as [];
newtype Z as mixed = int;

//// fileB.php
<?hh

function f()[ABC, Z]: void {}
