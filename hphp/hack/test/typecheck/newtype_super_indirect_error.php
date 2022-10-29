////file1.php
<?hh

<<file:__EnableUnstableFeatures('newtype_super_bounds')>>
newtype X super num = num;

////file2.php
<?hh

<<file:__EnableUnstableFeatures('newtype_super_bounds')>>
newtype Ok1 as X = X;
newtype Ok2 as X = float;
newtype Bad as X = bool;
