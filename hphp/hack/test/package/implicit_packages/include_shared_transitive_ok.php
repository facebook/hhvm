//// shared/s.php
<?hh
// belongs to package shared
const int SHARED_C = 1;

//// prototypes/alpha/a.php
<?hh
// prototypes.alpha includes shared (also reachable transitively via intern):
// access is allowed
const int ALPHA_C = SHARED_C;
