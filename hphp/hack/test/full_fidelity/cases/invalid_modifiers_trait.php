<?hh

// We can't write many of the modifiers or this won't parse as a trait (yet).
// Eventually we want to be testing:
// abstract final static private protected public async coroutine trait C {}
abstract final trait C {}

abstract abstract trait D {}
