/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_PERF_EVENTS_H_
#define incl_HPHP_PERF_EVENTS_H_

#include <algorithm>
#include <array>
#include <assert.h>
#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <folly/Format.h>
#include <folly/Optional.h>

#include "hphp/util/assertions.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace jit {

// If a new event type is added, please add the corresponding event
// caption in perf-events.cpp and tc-prod-print.sh.
enum PerfEventType {
  EVENT_CYCLES,
  EVENT_BRANCH_MISSES,
  EVENT_ICACHE_MISSES,
  EVENT_DCACHE_MISSES,
  EVENT_LLC_MISSES,
  EVENT_ITLB_MISSES,
  EVENT_DTLB_MISSES,
  SPECIAL_PROF_COUNTERS,
  NUM_EVENT_TYPES
};

PerfEventType commandLineArgumentToEventType(const char* argument);
PerfEventType perfScriptOutputToEventType(const char* eventId);
const char* eventTypeToCommandLineArgument(PerfEventType eventType);

struct PerfEvent {
  PerfEventType type;
  uint64_t      count;
};

template<typename S, typename T>
class Mapper {
public:
  virtual ~Mapper() {}
  virtual folly::Optional<T> operator()(const S&) = 0;
};

class StackTraceTree {
  struct Node;

  std::shared_ptr<Node> root;

  void printNode(const Node* node,
                 double minPercentage,
                 uint32_t depth,
                 std::vector<uint32_t>& activeColumns,
                 uint64_t kEventCount) const;

  void aggregate(const Node* from, Node* to);

public:
  StackTraceTree();

  void insert(const std::vector<std::string>& trace, uint64_t count);
  void printTop(double minPercentage, uint64_t kEventCount) const;
  void aggregateTree(const StackTraceTree& other);
};

template<typename KeyType>
class PerfEventsMap {

public:
  typedef std::array<uint64_t, NUM_EVENT_TYPES> EventsArray;

private:
  typedef std::map<KeyType, EventsArray> MapType;
  MapType eventsMap;

  typedef std::array<StackTraceTree, NUM_EVENT_TYPES> ArrayOfTrees;
  typedef std::map<KeyType, ArrayOfTrees> KeyToTreesMap;
  KeyToTreesMap treesMap;

  template<typename C>
  void outputAligned(C val) const {
    std::cout << folly::format("{:>20}", val);
  }

  void printColumnHeaders(std::string keyColName,
                          const std::vector<PerfEventType>& etypes) const {
    outputAligned(keyColName);
    std::cout << " ";
    for (size_t i = 0; i < etypes.size(); i++) {
      outputAligned(eventTypeToCommandLineArgument(etypes[i]));
      std::cout << " ";
    }
    std::cout << "\n";
  }

  void printTopHelpers(KeyType key,
                       PerfEventType etype,
                       double minPercentage,
                       uint64_t eventTotalCount) const {

    typename KeyToTreesMap::const_iterator it = treesMap.find(key);
    uint64_t eventCount = getEventCount(key, etype);

    if (it != treesMap.end() && eventCount) {
      std::cout << "\n";
      it->second[etype].printTop(minPercentage, eventTotalCount);
      std::cout << "\n";
    }
  }

  typename MapType::iterator getInitialized(KeyType key) {
    typename MapType::iterator it;
    it = eventsMap.insert(std::pair<KeyType, EventsArray>(key, EventsArray()))
                  .first;
    it->second.fill(0);
    return it;
  }

public:
  static const size_t kAllEntries = (size_t)-1;

  typedef typename MapType::const_iterator const_iterator;
  const_iterator begin() const { return eventsMap.begin(); }
  const_iterator end() const { return eventsMap.end(); }

  bool empty() const { return eventsMap.empty(); }

  void addEvent(KeyType key,
                const PerfEvent &event,
                const std::vector<std::string>& trace=
                  std::vector<std::string>()) {

    assert(event.type < NUM_EVENT_TYPES);

    typename MapType::iterator it = eventsMap.find(key);
    if (it == eventsMap.end()) it = getInitialized(key);

    it->second[event.type] += event.count;
    treesMap[key][event.type].insert(trace, event.count);
  }

  void addEvents(KeyType key, const EventsArray& events) {
    typename MapType::iterator it = eventsMap.find(key);
    if (it == eventsMap.end()) it = getInitialized(key);
    for (size_t i = 0; i < NUM_EVENT_TYPES; i++) it->second[i] += events[i];
  }

  uint64_t getEventCount(KeyType key, PerfEventType type) const {
    uint64_t eventCount = 0;

    typename MapType::const_iterator it = eventsMap.find(key);
    if (it != eventsMap.end()) {
      eventCount = it->second[type];
    }

    return eventCount;
  }

  /*
   * Returns the number of events of the specified type occurring in the
   * *closed* interval [low, high].
   */
  uint64_t getEventCount(KeyType low, KeyType high, PerfEventType type) const {
    assert(low <= high && type < NUM_EVENT_TYPES);
    uint64_t eventCount = 0;

    typename MapType::const_iterator itLow, itHigh;
    itLow = eventsMap.lower_bound(low);
    itHigh = eventsMap.upper_bound(high);

    for (; itLow != itHigh; itLow++) eventCount += itLow->second[type];
    return eventCount;
  }

  EventsArray getAllEvents(KeyType key) const {
    typename MapType::const_iterator it = eventsMap.find(key);
    if (it == eventsMap.end()) {
      EventsArray ret;
      ret.fill(0);
      return ret;
    }
    return it->second;
  }

  void aggregateStackTraceTree(KeyType key,
                               PerfEventType etype,
                               const StackTraceTree& tree) {

    treesMap[key][etype].aggregateTree(tree);
  }

  /*
   * Abstracts the following pattern: given a PerfEventsMap<X>, extract all its
   * keys, map them to another key type Y (e.g. map TCAs to TransIDs) and
   * create a new PerfEventsMap<Y>.
   */
  template<typename NewKeyType>
  PerfEventsMap<NewKeyType> mapTo(Mapper<KeyType, NewKeyType>& mapper) const {
    PerfEventsMap<NewKeyType> newMap;

    for (const_iterator itEvents = begin(); itEvents != end(); itEvents++) {
      folly::Optional<NewKeyType> newKey = mapper(itEvents->first);
      if (!newKey) continue;
      newMap.addEvents(*newKey, getAllEvents(itEvents->first));

      typename KeyToTreesMap::const_iterator
        itTrees = treesMap.find(itEvents->first);

      if (itTrees != treesMap.end()) {
        for (size_t i = 0; i < NUM_EVENT_TYPES; i++) {
          newMap.aggregateStackTraceTree(*newKey,
                                         (PerfEventType)i,
                                         itTrees->second[i]);
        }
      }
    }

    return newMap;
  }

  void printEventsSummary(PerfEventType sortBy,
                          std::string keyColName,
                          size_t topN=kAllEntries,
                          bool verbose=false,
                          double minPercentage=0,
                          std::map<KeyType, std::string>
                          keyToStr = (std::map<KeyType,std::string>())) const {

    const size_t kErrBuffSize = 100;
    static char errBuff[kErrBuffSize];

    always_assert(sortBy < NUM_EVENT_TYPES);

    // Sort the keys by the appropriate event.
    std::vector<std::pair<uint64_t, KeyType> > ranking;
    for (const_iterator it = begin(); it != end(); it++) {
      ranking.push_back(
        std::pair<uint64_t, KeyType>(it->second[sortBy], it->first)
      );
    }
    std::sort(ranking.rbegin(), ranking.rend());

    // Calculate the totals.
    EventsArray eventTotals;
    eventTotals.fill(0);
    for (const_iterator it = begin(); it != end(); it++) {
      for (size_t i = 0; i < NUM_EVENT_TYPES; i++) {
        eventTotals[i] += it->second[i];
      }
    }

    // We only summarize events that are non-zero for some key.
    std::vector<PerfEventType> printedEvents;
    printedEvents.push_back(sortBy);
    for (size_t i = 0; i < NUM_EVENT_TYPES; i++) {
      if (i == sortBy || !eventTotals[i]) continue;
      printedEvents.push_back((PerfEventType)i);
    }

    if (!verbose) {
      // Otherwise, we print them multilple times below.
      printColumnHeaders(keyColName, printedEvents);
    }

    // Print the rows: one per key.
    topN = std::min(topN, ranking.size());
    for (size_t i = 0; i < topN; i++) {

      if (verbose) printColumnHeaders(keyColName, printedEvents);

      // Try to map the key, if possible.
      if (keyToStr.empty()) {
        outputAligned(ranking[i].second);
      } else {
        auto it = keyToStr.find(ranking[i].second);
        always_assert(it != keyToStr.end());
        outputAligned(it->second);
      }
      std::cout << " ";

      // And now all the events.
      for (size_t j = 0; j < printedEvents.size(); j++) {
        uint64_t eventCount = getEventCount(ranking[i].second, printedEvents[j]);

        // sortBy could still be 0
        auto total = std::max<uint64_t>(eventTotals[printedEvents[j]], 1);
        snprintf(errBuff,
                 kErrBuffSize,
                 "%10" PRIu64 " (%5.2lf%%)",
                 eventCount,
                 eventCount * 100.0 / total);
        outputAligned(errBuff);
        std::cout << " ";
      }
      std::cout << "\n";

      if (verbose) {
        printTopHelpers(ranking[i].second,
                        sortBy,
                        minPercentage,
                        eventTotals[sortBy]);
      }
    }
  }

  void printEventsHeader(KeyType key) const {
    EventsArray events = getAllEvents(key);

    bool anyEvents = false;
    for (size_t i = 0; i < SPECIAL_PROF_COUNTERS; i++) {
      if (events[i]) {
        anyEvents = true;
        break;
      }
    }

    if (anyEvents) {
      printf("  == Perf events ==\n");

      for (size_t i = 0; i < SPECIAL_PROF_COUNTERS; i++) {
        if (events[i]) {
          printf("  %-16s = %" PRIu64  "\n",
                 eventTypeToCommandLineArgument((PerfEventType)i),
                 events[i]);
        }
      }
    }
  }
};

} }
#endif
