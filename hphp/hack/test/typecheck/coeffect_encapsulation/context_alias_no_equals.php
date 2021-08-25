//// f.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>

// Codegen framework
newctx Codegen as [];

function enter_codegen(
  (function ()[\Codegen]: void) $f
): void {
  echo "Hello I can secretly do IO";
  $f();
}

//// g.php
<?hh

// client code
<<__EntryPoint>>
function main(): void {
  enter_codegen(
    () ==> {
      echo "break purity"; // illegal!
    }
  );
}
