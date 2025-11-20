//// foo.php
<?hh

// package pkg4 (disjoint from pkg1)
<<file: __PackageOverride('pkg4')>>

abstract enum class C : I {}

//// bar.php
<?hh
// package pkg1

class I {}

class D {}
