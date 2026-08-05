//// prototypes/alpha/a.php
<?hh
// belongs to synthesized package prototypes.alpha
const int ALPHA_C = 1;

//// prototypes/alpha/deep/d.php
<?hh
// membership is the first segment under //prototypes/, so a deeply nested file
// still belongs to prototypes.alpha: same-package access is allowed
const int DEEP_C = ALPHA_C;

//// prototypes/beta/b.php
<?hh
// prototypes.beta cannot access prototypes.alpha, even a nested file of it
const int BETA_C = DEEP_C;
