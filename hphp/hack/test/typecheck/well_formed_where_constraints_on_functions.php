<?hh

class G<T as arraykey> {}

function on_g<T>(T $_): void where T = G<null> {}
