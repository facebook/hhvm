//// base-a.php
<?hh

class A {
  const int C = 0;
}

//// base-b.php
<?hh

class B extends A {}

//// base-c.php
<?hh

class C extends B {}

//// base-d.php
<?hh

class D extends C {
  const int C = 0;
}

//// base-e.php
<?hh

class E extends C {}

//// base-f.php
<?hh

class F extends E {
  const int C = 0;
}

//// base-g.php
<?hh

class G extends F {}

//// base-use-b.php
<?hh

function f_b(): void {
  B::C;
}

//// base-use-c.php
<?hh

function f_c(): void {
  C::C;
}

//// base-use-d.php
<?hh

function f_d(): void {
  D::C;
}
//// base-use-g.php
<?hh

function f_g(): void {
  F::C;
}

//// changed-a.php
<?hh

class A {
  const int C = 0;
}

//// changed-b.php
<?hh

class B extends A {
  const string C = "";
}

//// changed-c.php
<?hh

class C extends B {}

//// changed-d.php
<?hh

class D extends C {
  const int C = 0;
}

//// changed-e.php
<?hh

class E extends C {}

//// changed-f.php
<?hh

class F extends E {
  const int C = 0;
}

//// changed-g.php
<?hh

class G extends F {}

//// changed-use-b.php
<?hh

function f_b(): void {
  B::C;
}

//// changed-use-c.php
<?hh

function f_c(): void {
  C::C;
}

//// changed-use-d.php
<?hh

function f_d(): void {
  D::C;
}
//// changed-use-g.php
<?hh

function f_g(): void {
  F::C;
}
