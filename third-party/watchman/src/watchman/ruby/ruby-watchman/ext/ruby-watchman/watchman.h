/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <ruby.h> // @manual

/**
 * @module RubyWatchman
 *
 * Methods for working with the Watchman binary protocol
 *
 * @see https://facebook.github.io/watchman/docs/bser.html
 */
extern VALUE mRubyWatchman;

/**
 * Convert an object serialized using the Watchman binary protocol[0] into an
 * unpacked Ruby object
 */
extern VALUE RubyWatchman_load(VALUE self, VALUE serialized);

/**
 * Serialize a Ruby object into the Watchman binary protocol format
 */
extern VALUE RubyWatchman_dump(VALUE self, VALUE serializable);

/**
 * Issue `query` to the Watchman instance listening on `socket` (a `UNIXSocket`
 * instance) and return the result
 *
 * The query is serialized following the Watchman binary protocol and the
 * result is converted to native Ruby objects before returning to the caller.
 */
extern VALUE RubyWatchman_query(VALUE self, VALUE query, VALUE socket);
