<?hh // decl
// Copyright 2004-present Facebook. All Rights Reserved.

namespace HH\Asio {

function m<Tk, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<Map<Tk, Tv>>;

function v<Tv>(
  Traversable<Awaitable<Tv>> $awaitables,
): Awaitable<Vector<Tv>>;

} // namespace HH\Asio
