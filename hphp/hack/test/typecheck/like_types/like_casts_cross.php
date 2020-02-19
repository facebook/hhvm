//// file1.php
<?hh

newtype HideInt = int;
newtype HideT<T> = T;
type HideInt2 = HideT<int>;

//// file2.php
<?hh

const HH\INCORRECT_TYPE<HideInt> T = 3 as HH\INCORRECT_TYPE<HideInt>;
const HH\INCORRECT_TYPE<HideInt2> T2 = 3 as HH\INCORRECT_TYPE<HideInt2>;

// make sure like cast type is still checked
const mixed Bad = 3 as HH\INCORRECT_TYPE<(function (): void)>;
