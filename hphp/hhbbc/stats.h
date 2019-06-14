/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#ifndef incl_HHBBC_STATS_H_
#define incl_HHBBC_STATS_H_

namespace HPHP { namespace HHBBC {

struct Stats;

struct StatsHolder {
  StatsHolder();
  ~StatsHolder();
  StatsHolder(StatsHolder&& o) noexcept : stats{o.stats} { o.stats = nullptr; }
  StatsHolder(const StatsHolder&) = delete;
  StatsHolder& operator=(const StatsHolder&) = delete;
  StatsHolder& operator=(StatsHolder&&) = delete;

  operator bool() const { return stats; }

  Stats* stats{};
};

namespace php {
struct Program;
struct Unit;
}
struct Index;

//////////////////////////////////////////////////////////////////////

/*
 * If Trace::hhbbc_time >= 1, print some stats about the program to a
 * temporary file.  If it's greater than or equal to 2, also dump it
 * to stdout.
 */
StatsHolder allocate_stats();
void collect_stats(const StatsHolder&, const Index&, const php::Unit*);
void print_stats(const StatsHolder&);

//////////////////////////////////////////////////////////////////////

}}

#endif
