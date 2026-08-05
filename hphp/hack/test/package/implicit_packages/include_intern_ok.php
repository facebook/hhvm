//// intern/i.php
<?hh
// belongs to package intern
const int INTERN_C = 1;

//// prototypes/alpha/a.php
<?hh
// prototypes.alpha hard-includes intern: access is allowed
const int ALPHA_C = INTERN_C;
