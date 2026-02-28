<?hh

function f(): ~int {}

async function af(): Awaitable<~int> {}

function implicit_return() : dynamic {}

async function async_implicit_return(): Awaitable<dynamic> {}
