//// pkg4/foo.php
<?hh

// package pkg4 (disjoint from pkg1) by path

abstract enum class C : I {}

//// bar.php
<?hh
// package pkg1

class I {}

class D {}
