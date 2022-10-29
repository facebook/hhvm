////file1.php
<?hh

<<file: __EnableUnstableFeatures('context_alias_declaration_short')>>
newctx MyContext as [write_props] super [defaults];

////file2.php
<?hh

function foo()[MyContext]: void {
  baz(() ==> foo());
}

function bar()[MyContext]: void {
  baz(()[MyContext] ==> bar());
}

function baz((function()[_]: void) $p)[ctx $p]: void {
  $p();
}
