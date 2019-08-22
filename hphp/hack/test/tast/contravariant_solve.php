<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I<-T> {}

class E<T> implements I<T> {}

/* HH_FIXME[4336] */
function f<T>(I<T> $_, I<T> $_): I<T> {
}

/* HH_FIXME[4336] */
function g<T>(T $_): E<T> {
}

function test(): void {
  $_ = f(g(1), g(2));
}
