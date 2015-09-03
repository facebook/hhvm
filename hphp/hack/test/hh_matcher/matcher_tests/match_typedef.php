//// tosearch.php
<?hh //strict
newtype MyType = int;
newtype MyType2 = (string, float);

//// matcherpattern.php
<?hh //strict
newtype __ANY = int;
newtype MyType2 = (__ANY, float);
