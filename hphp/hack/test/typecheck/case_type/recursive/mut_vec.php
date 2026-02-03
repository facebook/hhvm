//// f1.php
<?hh

<<file: __EnableUnstableFeatures('recursive_case_types')>>
case type X = null | vec<Y>;
//// f2.php
<?hh

<<file: __EnableUnstableFeatures('recursive_case_types')>>
case type Y = X;
