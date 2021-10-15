//// f.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newctx First as [] = [defaults];

//// g.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newctx Second as [First] = [defaults, First];

function int_op((function (int)[First]: int) $f, int $i)[First]: void {
  $result = $f($i);
}

//// h.php
<?hh

// client code
<<__EntryPoint>>
function main()[Second]: void {
  int_op((int $inp) ==> {
    return $inp + 4;
  }, 5);
}
