//// modules.php
<?hh


new module cookies {}

//// test.php
<?hh

module cookies;


internal interface I {}

class B implements I {}
