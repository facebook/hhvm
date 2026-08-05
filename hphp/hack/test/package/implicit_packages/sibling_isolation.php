//// prototypes/alpha/a.php
<?hh
// belongs to synthesized package prototypes.alpha
const int ALPHA_C = 1;

//// prototypes/beta/b.php
<?hh
// belongs to synthesized package prototypes.beta
// prototypes.beta does not include prototypes.alpha: cross-package violation
const int BETA_C = ALPHA_C;
