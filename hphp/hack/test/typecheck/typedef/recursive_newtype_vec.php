//// f1.php
<?hh

newtype N1 = ?N2;
//// f2.php
<?hh

newtype N2 = vec<N1>;
