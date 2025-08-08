/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/CPortability.h>

/**
 * C++ switch-case statements enable writing conditional statements that are
 * checked for exhaustiveness by the compiler.  However, C++ compilers don't
 * enforce exhaustiveness very well.  These macros enable you to explicitly opt
 * into truly exhaustive switch-cases, that are useful for scenarios where you
 * may want to deal with enums that are inputs, rather than internal invariants.
 *
 * Historically, we have encountered one too many instances of SEVs caused by
 * improper handling of enums, so these macros are used to enable stronger, and
 * more explicit guarantees when writing switch-cases.
 *
 * For example:
 *
 *    enum class logposition_t {
 *      sealed = -1,
 *      nonexistent = -2,
 *    };
 *
 *    void foo(logposition_t logpos) {
 *      FOLLY_EXHAUSTIVE_SWITCH({
 *        switch (logpos) {
 *          case logposition_t::sealed:
 *            // handle sealed case
 *          case logposition_t::nonexistent:
 *            // handle nonexistent case
 *          default:
 *            // handle non-exceptional value
 *        }
 *      });
 *    }
 *
 * For the above, if you miss handling any case, or if you miss the default
 * case, the compiler will complain.
 *
 * When you switch on the value of a concretely defined enum with a very large
 * set of values, and only need to address a sane subset of the values, you
 * write a flexible switch. Additionally, when you switch on the a non-enum
 * value (like an int), you write a flexible switch.
 *
 *    enum class Color {
 *      // every color in a 64 color palette named as an enum value
 *    };
 *
 *    bool isRedColor(Color c) {
 *      FOLLY_FLEXIBLE_SWITCH({
 *        switch (c) {
 *          case Color::Red:
 *          case Color::LightRed:
 *          case Color::DarkRed:
 *            return true;
 *          default:
 *            return false;
 *        }
 *      });
 *    }
 *
 * The above is the less common pattern for switch statements.
 *
 * Note: The two macros here do not work well before llvm-18, they are known to
 * work for most recent versions of gcc, however.
 */
#define FOLLY_EXHAUSTIVE_SWITCH(...)               \
  FOLLY_PUSH_WARNING                               \
  FOLLY_GNU_DISABLE_COVERED_SWITCH_DEFAULT_WARNING \
  FOLLY_GNU_ENABLE_ERROR("-Wswitch-default")       \
  FOLLY_GNU_ENABLE_ERROR("-Wswitch-enum")          \
  FOLLY_GNU_ENABLE_ERROR("-Wswitch")               \
  __VA_ARGS__                                      \
  FOLLY_POP_WARNING                                \
  static_assert(true, "Add a semicolon after this line for formatting")

#define FOLLY_FLEXIBLE_SWITCH(...)              \
  FOLLY_PUSH_WARNING                            \
  FOLLY_GNU_DISABLE_WARNING("-Wswitch-enum")    \
  FOLLY_GNU_ENABLE_COVERED_SWITCH_DEFAULT_ERROR \
  FOLLY_GNU_ENABLE_ERROR("-Wswitch-default")    \
  FOLLY_GNU_ENABLE_ERROR("-Wswitch")            \
  __VA_ARGS__                                   \
  FOLLY_POP_WARNING                             \
  static_assert(true, "Add a semicolon after this line for formatting")

#if defined(__has_warning) && (defined(__GNUC__) || defined(__clang__))
#if __has_warning("-Wcovered-switch-default")
#define FOLLY_GNU_DISABLE_COVERED_SWITCH_DEFAULT_WARNING \
  FOLLY_GNU_DISABLE_WARNING("-Wcovered-switch-default")
#define FOLLY_GNU_ENABLE_COVERED_SWITCH_DEFAULT_ERROR \
  FOLLY_GNU_ENABLE_ERROR("-Wcovered-switch-default")
#endif
#endif

#ifndef FOLLY_GNU_DISABLE_COVERED_SWITCH_DEFAULT_WARNING
#define FOLLY_GNU_DISABLE_COVERED_SWITCH_DEFAULT_WARNING
#define FOLLY_GNU_ENABLE_COVERED_SWITCH_DEFAULT_ERROR
#endif
