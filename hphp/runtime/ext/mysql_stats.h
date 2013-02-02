/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef __HPHP_MYSQL_STATS_H__
#define __HPHP_MYSQL_STATS_H__

#include <util/base.h>
#include <util/lock.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class MySqlStats {
public:
  enum Verb {
    UNKNOWN,

    T_BEGIN,
    T_COMMIT,
    T_ROLLBACK,

    // transactional: statements inside begin/commit AND at least 2 statements
    X_INSERT,
    X_UPDATE,
    X_INCDEC,
    X_DELETE,
    X_REPLACE,
    X_SELECT,

    // transactional: statements inside begin/commit
    T_INSERT,
    T_UPDATE,
    T_INCDEC,
    T_DELETE,
    T_REPLACE,
    T_SELECT,

    // non-transactional
    N_INSERT,
    N_UPDATE,
    N_INCDEC,
    N_DELETE,
    N_REPLACE,
    N_SELECT,

    VERB_COUNT
  };

public:
  static void Record(const std::string &verb, int xactionCount = 0,
                     const std::string &table = "");
  static std::string ReportStats();

private:
  DECLARE_BOOST_TYPES(Stats);
  struct Stats {
    int actions[VERB_COUNT];
  };
  typedef hphp_string_map<StatsPtr> StatsMap;

  static bool s_inited;
  static hphp_string_map<Verb> s_verbs;
  static const char *s_verb_names[];

  static Mutex s_mutex;
  static StatsMap s_stats;

  static void Init();
  static Verb Translate(const std::string &verb, int xactionCount);
  static const char *Translate(Verb verb);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_MYSQL_STATS_H__
