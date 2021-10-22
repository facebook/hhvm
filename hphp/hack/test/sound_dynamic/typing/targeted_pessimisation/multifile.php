//// file1.php
<?hh

<<__SupportDynamicType>>
function f(shape() $s): void {
  g($s);
}

function g(shape() $s): void {}
//// file2.php
<?hh

<<__SupportDynamicType>>
function ff(shape() $s): void {
  gg($s);
}

function gg(shape() $s): void {}
