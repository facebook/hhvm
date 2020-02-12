//// file1.php
<?hh

newtype HideInt = int;

//// file2.php
<?hh

const HH\INCORRECT_TYPE<HideInt> T = 3 as HH\INCORRECT_TYPE<HideInt>;

// make sure like cast type is still checked
const mixed Bad = 3 as HH\INCORRECT_TYPE<(function (): void)>;
