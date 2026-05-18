//// base-a.php
<?hh

trait A { require class C; }
//// base-b.php
<?hh
final class C { use A; }

//// changed-a.php
<?hh

trait A { require class C; }
//// changed-b.php
<?hh
final class C {}
