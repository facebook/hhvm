<?hh

class A {}
$cb = meth_caller('B', 'c');
$cb(new A);
