//// base-decls.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

module A {}
module B {}

//// changed-decls.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

module A {}
module B {}

//// base-f.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
function f(): void {}

//// changed-f.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('B')>>

<<__Internal>>
function f(): void {}

//// base-g.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

function g(): void {
  f();
}

//// changed-g.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

function g(): void {
    f();
}
