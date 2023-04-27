// Copyright 2004-present Facebook. All Rights Reserved.

#include <deque>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "field.h"
#include "query_tag_perf_counter.h"
#include "sql_class.h"
#include "sql_show.h"
#include "table.h"

namespace qutils {

using cpu_and_num_queries = std::tuple<uint64_t, uint64_t>;
static int64_t timespec_diff(const timespec &start, const timespec &stop);

static std::mutex stats_mutex;
static std::unordered_map<std::string, cpu_and_num_queries> stats;

query_tag_perf_counter::query_tag_perf_counter(THD *_thd)
    : started(false), thd(_thd) {
  if (!thd->query_type.empty() && thd->num_queries > 0) {
    this->started =
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &this->starttime) == 0;
  }
}

query_tag_perf_counter::~query_tag_perf_counter() {
  if (!this->started) return;
  struct timespec endtime;
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &endtime) != 0) return;

  const int64_t cputime = timespec_diff(this->starttime, endtime);
  if (cputime < 0) return;  // skip if overflow

  DBUG_ASSERT(thd->num_queries > 0);

  std::lock_guard<std::mutex> lock(stats_mutex);
  cpu_and_num_queries &val = stats[thd->query_type];
  std::get<0>(val) += cputime;
  std::get<1>(val) += thd->num_queries;
}

int fill_query_tag_perf_counter(THD *thd, TABLE_LIST *tables, Item *) {
  std::lock_guard<std::mutex> lock(stats_mutex);

  DBUG_ENTER("fill_query_tag_perf_counter");

  TABLE *table = tables->table;
  for (const auto &row : stats) {
    restore_record(table, s->default_values);
    Field **field = table->field;

    const std::string &query_type = row.first;
    const cpu_and_num_queries &val = row.second;
    const uint64_t cpu_time = std::get<0>(val);
    const uint64_t num_queries = std::get<1>(val);

    field[0]->store(query_type.c_str(), query_type.length(),
                    system_charset_info);
    field[1]->store(cpu_time, true);
    field[2]->store(num_queries, true);

    if (schema_table_store_record(thd, table)) {
      DBUG_RETURN(-1);
    }
  }
  DBUG_RETURN(0);
}

const uint query_type_field_length = 254;

ST_FIELD_INFO query_tag_perf_fields_info[] = {
    {"QUERY_TYPE", query_type_field_length, MYSQL_TYPE_STRING, 0, 0, 0, 0},
    {"CPU", MY_INT64_NUM_DECIMAL_DIGITS, MYSQL_TYPE_LONGLONG, 0, 0, 0, 0},
    {"NUM_QUERIES", MY_INT64_NUM_DECIMAL_DIGITS, MYSQL_TYPE_LONGLONG, 0, 0, 0,
     0},
    {0, 0, MYSQL_TYPE_STRING, 0, 0, 0, 0}};

static int64_t timespec_diff(const timespec &start, const timespec &stop) {
  const int64_t sec = stop.tv_sec - start.tv_sec;
  const int64_t nsec = stop.tv_nsec - start.tv_nsec;
  const int64_t diff = sec * 1000000000LL + nsec;
  return diff;
}
}  // namespace qutils
