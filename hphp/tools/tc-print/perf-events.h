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

#include "hphp/util/assertions.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace jit {

// Enumerates all of the predefined event types. Any replacement event types
// passed with -E will start numbering at NUM_PREDEFINED_EVENT_TYPES. Change
// MAX_NUM_EVENT_TYPES if you need more than 12 (20 - 8 predefined) replacements
// events in one run.
enum PerfEventType {
  EVENT_NULL = -1,
  EVENT_CYCLES = 0,
  EVENT_BRANCH_MISSES,
  EVENT_ICACHE_MISSES,
  EVENT_DCACHE_MISSES,
  EVENT_LLC_MISSES,
  EVENT_LLC_STORE_MISSES,
  EVENT_ITLB_MISSES,
  EVENT_DTLB_MISSES,
  NUM_PREDEFINED_EVENT_TYPES,
  MAX_NUM_EVENT_TYPES = 20
};

size_t getFirstEventType();
size_t getNumEventTypes();
void addEventType(std::string eventId);
PerfEventType commandLineArgumentToEventType(const char* argument);
PerfEventType perfScriptOutputToEventType(std::string eventId);
const char* eventTypeToCommandLineArgument(PerfEventType eventType);
const char* eventTypeToSmallCaption(PerfEventType eventType);

struct PerfEvent {
  PerfEventType type;
  uint64_t      count;
};

template<typename S, typename T>
struct Mapper {
  virtual ~Mapper() {}
  virtual Optional<T> operator()(const S&) = 0;
};

struct StackTraceTree {
private:
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
struct PerfEventsMap {
  typedef std::array<uint64_t, MAX_NUM_EVENT_TYPES> EventsArray;

private:
  typedef std::map<KeyType, EventsArray> MapType;
  MapType eventsMap;

  typedef std::array<StackTraceTree, MAX_NUM_EVENT_TYPES> ArrayOfTrees;
  typedef std::map<KeyType, ArrayOfTrees> KeyToTreesMap;
  KeyToTreesMap treesMap;

  template<typename C>
  void outputAligned(C val) const {
    std::cout << folly::format("{:>16}", val);
  }

  template<typename C>
  void outputAlignedKey(C val, uint16_t width) const {
    std::cout << folly::format("{:<*}", width, val);
  }

  void printColumnHeaders(std::string keyColName, uint16_t keyColWidth,
                          const std::vector<PerfEventType>& etypes) const {
    outputAlignedKey(keyColName, keyColWidth);
    std::cout << " ";
    for (size_t i = 0; i < etypes.size(); i++) {
      std::string header(eventTypeToCommandLineArgument(etypes[i]));
      header.append("(");
      header.append(eventTypeToSmallCaption(etypes[i]));
      header.append(")");
      outputAligned(header.c_str());
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

    assert(event.type != EVENT_NULL);
    assert(event.type < MAX_NUM_EVENT_TYPES);

    typename MapType::iterator it = eventsMap.find(key);
    if (it == eventsMap.end()) it = getInitialized(key);

    it->second[event.type] += event.count;
    treesMap[key][event.type].insert(trace, event.count);
  }

  void addEvents(KeyType key, const EventsArray& events) {
    typename MapType::iterator it = eventsMap.find(key);
    if (it == eventsMap.end()) it = getInitialized(key);
    for (size_t i = 0; i < MAX_NUM_EVENT_TYPES; i++) it->second[i] += events[i];
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
    assert(low <= high && type != EVENT_NULL && type < MAX_NUM_EVENT_TYPES);
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
      Optional<NewKeyType> newKey = mapper(itEvents->first);
      if (!newKey) continue;
      newMap.addEvents(*newKey, getAllEvents(itEvents->first));

      typename KeyToTreesMap::const_iterator
        itTrees = treesMap.find(itEvents->first);

      if (itTrees != treesMap.end()) {
        for (size_t i = 0; i < MAX_NUM_EVENT_TYPES; i++) {
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
                          uint16_t keyColWidth,
                          size_t topN=kAllEntries,
                          bool verbose=false,
                          double minPercentage=0,
                          std::map<KeyType, std::string>
                          keyToStr = (std::map<KeyType,std::string>())) const {

    const size_t kErrBuffSize = 100;
    static char errBuff[kErrBuffSize];

    always_assert(sortBy != EVENT_NULL && sortBy < MAX_NUM_EVENT_TYPES);

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
      for (size_t i = 0; i < MAX_NUM_EVENT_TYPES; i++) {
        eventTotals[i] += it->second[i];
      }
    }

    // We only summarize events that are non-zero for some key.
    std::vector<PerfEventType> printedEvents;
    printedEvents.push_back(sortBy);
    for (size_t i = 0; i < MAX_NUM_EVENT_TYPES; i++) {
      if (i == sortBy || !eventTotals[i]) continue;
      printedEvents.push_back((PerfEventType)i);
    }

    if (!verbose) {
      // Otherwise, we print them multiple times below.
      printColumnHeaders(keyColName, keyColWidth, printedEvents);
    }

    // Print the rows: one per key.
    topN = std::min(topN, ranking.size());
    for (size_t i = 0; i < topN; i++) {

      if (verbose) printColumnHeaders(keyColName, keyColWidth, printedEvents);

      // Try to map the key, if possible.
      if (keyToStr.empty()) {
        outputAlignedKey(ranking[i].second, keyColWidth);
      } else {
        auto it = keyToStr.find(ranking[i].second);
        always_assert(it != keyToStr.end());
        outputAlignedKey(it->second, keyColWidth);
      }
      std::cout << " ";

      // And now all the events.
      for (size_t j = 0; j < printedEvents.size(); j++) {
        uint64_t eventCount = getEventCount(ranking[i].second, printedEvents[j]);

        // sortBy could still be 0
        auto total = std::max<uint64_t>(eventTotals[printedEvents[j]], 1);
        snprintf(errBuff,
                 kErrBuffSize,
                 "%6" PRIu64 " (%5.2lf%%)",
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
    for (size_t i = 0; i < MAX_NUM_EVENT_TYPES; i++) {
      if (events[i]) {
        anyEvents = true;
        break;
      }
    }

    if (anyEvents) {
      printf("  == Perf events ==\n");

      for (size_t i = 0; i < MAX_NUM_EVENT_TYPES; i++) {
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
