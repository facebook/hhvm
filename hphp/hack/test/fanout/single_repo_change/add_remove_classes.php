//// base-a.php
<?hh

//// base-b.php
<?hh
class B {}

//// base-f.php
<?hh

function f(): void {
  new A();
}

//// base-g.php
<?hh

function g(): void {
  new B();
}

//// changed-a.php
<?hh
class A {}

//// changed-b.php
<?hh

//// changed-f.php
<?hh

function f(): void {
  new A();
}

//// changed-g.php
<?hh

function g(): void {
  new B();
}
