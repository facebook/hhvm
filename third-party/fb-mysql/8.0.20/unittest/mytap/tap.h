/* Copyright (c) 2006, 2018, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef TAP_H
#define TAP_H

#include "my_compiler.h"

/*
  @defgroup MyTAP MySQL support for performing unit tests according to
  the Test Anything Protocol (TAP).
*/

#define NO_PLAN (0)

/**
   Data about test plan.

   @ingroup MyTAP_Internal

   @internal We are using the "typedef struct X { ... } X" idiom to
   create class/struct X both in C and C++.
 */

typedef struct TEST_DATA {
  /**
     Number of tests that is planned to execute.

     Can be zero (<code>NO_PLAN</code>) meaning that the plan string
     will be printed at the end of test instead.
  */
  int plan;

  /** Number of last test that was done or skipped. */
  int last;

  /** Number of tests that failed. */
  int failed;

  /** Todo reason. */
  char todo[128];
} TEST_DATA;

#ifdef __cplusplus
extern "C" {
#endif

/**
   Defines whether "big" tests should be skipped.

   This variable is set by plan() function unless MYTAP_CONFIG environment
   variable is set to the string "big".  It is supposed to be used as

   @code
   if (skip_big_tests) {
     skip(1, "Big test skipped");
   } else {
     ok(life_universe_and_everything() == 42, "The answer is CORRECT");
   }
   @endcode

   @see SKIP_BIG_TESTS
*/
extern int skip_big_tests;

/**
  @defgroup MyTAP_API MyTAP API

  MySQL support for performing unit tests according to TAP.

  @{
*/

/**
   Set number of tests that is planned to execute.

   The function also accepts the predefined constant <code>NO_PLAN</code>.
   If invoked with this constant -- or not invoked at all --
   the test plan will be printed after all the test lines.

   The plan() function will install signal handlers for all signals
   that generate a core, so if you want to override these signals, do
   it <em>after</em> you have called the plan() function.

   It will also set skip_big_tests variable if MYTAP_CONFIG environment
   variable is defined.

   @see skip_big_tests

   @param count The planned number of tests to run.
*/

void plan(int const count);

/**
   Report test result as a TAP line.

   Function used to write status of an individual test.  Call this
   function in the following manner:

   @code
   ok(ducks == paddling,
      "%d ducks did not paddle", ducks - paddling);
   @endcode

   @param pass Zero if the test failed, non-zero if it passed.
   @param fmt  Format string in printf() format. NULL is not allowed,
               use ok1() in this case.
*/

void ok(int const pass, char const *fmt, ...)
    MY_ATTRIBUTE((format(printf, 2, 3)));

/**
   Report test result as a TAP line.

   Same as ok() but does not take a message to be printed.

   @param pass Zero if the test failed, non-zero if it passed.
*/

void ok1(int const pass);

/**
   Skip a determined number of tests.

   Function to print that <em>how_many</em> tests have been skipped.
   The reason is printed for each skipped test.  Observe that this
   function does not do the actual skipping for you, it just prints
   information that tests have been skipped. This function is not
   usually used, but rather the macro @c SKIP_BLOCK_IF, which does the
   skipping for you.

   It shall be used in the following manner:

   @code
   if (ducks == 0) {
     skip(2, "No ducks in the pond");
   } else {
      int i;
      for (i = 0 ; i < 2 ; ++i)
        ok(duck[i] == paddling, "is duck %d paddling?", i);
   }
   @endcode

   @see SKIP_BLOCK_IF

   @param how_many   Number of tests that are to be skipped.
   @param reason     A reason for skipping the tests
 */

void skip(int how_many, char const *reason, ...)
    MY_ATTRIBUTE((format(printf, 2, 3)));

/**
   Helper macro to skip a block of code.  The macro can be used to
   simplify conditionally skipping a block of code.  It is used in the
   following manner:

   @code
   SKIP_BLOCK_IF(ducks == 0, 2, "No ducks in the pond")
   {
     int i;
     for (i = 0 ; i < 2 ; ++i)
       ok(duck[i] == paddling, "is duck %d paddling?", i);
   }
   @endcode

   @see skip
 */

#define SKIP_BLOCK_IF(SKIP_IF_TRUE, COUNT, REASON) \
  if (SKIP_IF_TRUE)                                \
    skip((COUNT), (REASON));                       \
  else

/**
   Helper macro to skip a group of "big" tests. It is used in the following
   manner:

   @code
   SKIP_BIG_TESTS(1)
   {
     ok(life_universe_and_everything() == 42, "The answer is CORRECT");
   }
   @endcode

   @see skip_big_tests
 */

#define SKIP_BIG_TESTS(COUNT)  \
  if (skip_big_tests)          \
    skip((COUNT), "big test"); \
  else

/**
   Print a diagnostics message.

   @param fmt  Diagnostics message in printf() format.
 */

void diag(char const *fmt, ...) MY_ATTRIBUTE((format(printf, 1, 2)));

/**
   Print a bail out message.

   A bail out message can be issued when no further testing can be
   done, e.g., when there are missing dependencies.

   The test will exit with status 255.  This function does not return.

   @code
   BAIL_OUT("Lost connection to server %s", server_name);
   @endcode

   @note A bail out message is printed if a signal that generates a
   core is raised.

   @param fmt Bail out message in printf() format.
*/

void BAIL_OUT(char const *fmt, ...)
    MY_ATTRIBUTE((noreturn, format(printf, 1, 2)));

/**
   Print summary report and return exit status.

   This function will print a summary report of how many tests passed,
   how many were skipped, and how many remains to do.  The function
   should be called after all tests are executed in the following
   manner:

   @code
   return exit_status();
   @endcode

   @returns @c EXIT_SUCCESS if all tests passed, @c EXIT_FAILURE if
   one or more tests failed.
 */

int exit_status(void);

/**
   Skip entire test suite.

   To skip the entire test suite, use this function. It will
   automatically call exit(), so there is no need to have checks
   around it.
 */

void skip_all(char const *reason, ...)
    MY_ATTRIBUTE((noreturn, format(printf, 1, 2)));

/**
   Start section of tests that are not yet ready.

   To start a section of tests that are not ready and are expected to
   fail, use this function and todo_end() in the following manner:

   @code
   todo_start("Not ready yet");
   ok(is_rocketeering(duck), "Rocket-propelled ducks");
   ok(is_kamikaze(duck), "Kamikaze ducks");
   todo_end();
   @endcode

   @see todo_end

   @note
   It is not possible to nest todo sections.

   @param message Message that will be printed before the todo tests.
*/

void todo_start(char const *message, ...) MY_ATTRIBUTE((format(printf, 1, 2)));

/**
   End a section of tests that are not yet ready.
*/

void todo_end();

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* TAP_H */
