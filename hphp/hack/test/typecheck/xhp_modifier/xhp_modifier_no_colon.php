<?hh // strict

// XHP classes do no allow leading colons in class names.
xhp class :foo {}

// Colons elsewhere are fine.
xhp class foo:bar {}
