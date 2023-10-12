<?hh

function variadic(mixed ... $x) : void {}

function ref(int & $x) : void {}

function varvar(mixed ... ... $x) : void {}

function refref(int & & $x) : void {}

function varref(int ... & $x) : void {}

function refvar(mixed & ... $x) : void {}
