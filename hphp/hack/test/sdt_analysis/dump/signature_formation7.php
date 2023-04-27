<?hh

class C<T> {}

function needs_sdt<T>(C<T> $_): void {}

function doesnt_need_sdt<T as arraykey>(C<T> $_): void {}
