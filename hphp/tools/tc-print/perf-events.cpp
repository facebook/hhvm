/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/tools/tc-print/perf-events.h"

#include <unordered_map>

#include "hphp/tools/tc-print/tc-print.h"

namespace HPHP { namespace jit {

using std::string;

// PerfEvent

PerfEventType perfScriptOutputToEventType(const char* eventId) {

  // Events are duplicated so we can use the non-generic perf events.
  std::pair<const char*, PerfEventType> idToType[] = {
    std::make_pair("cycles",                    EVENT_CYCLES),
    std::make_pair("cpu-clock",                 EVENT_CYCLES),

    std::make_pair("branch-misses",             EVENT_BRANCH_MISSES),
    std::make_pair("0x5300c5",                  EVENT_BRANCH_MISSES),

    // fake event grouping all icache misses
    std::make_pair("L1-icache-misses",          EVENT_ICACHE_MISSES),
    // real icache miss events
    std::make_pair("L1-icache-load-misses",     EVENT_ICACHE_MISSES),
    std::make_pair("L1-icache-prefetch-misses", EVENT_ICACHE_MISSES),

    // fake event grouping all dcache misses
    std::make_pair("L1-dcache-misses",          EVENT_DCACHE_MISSES),
    // real dcache miss events
    std::make_pair("L1-dcache-load-misses",     EVENT_DCACHE_MISSES),
    std::make_pair("L1-dcache-store-misses",    EVENT_DCACHE_MISSES),
    std::make_pair("L1-dcache-prefetch-misses", EVENT_DCACHE_MISSES),

    std::make_pair("cache-misses",              EVENT_LLC_MISSES),

    // fake event grouping all iTLB misses
    std::make_pair("iTLB-misses",               EVENT_ITLB_MISSES),
    // real iTLB miss events
    std::make_pair("iTLB-load-misses",          EVENT_ITLB_MISSES),

    // fake event grouping all dTLB misses
    std::make_pair("dTLB-misses",               EVENT_DTLB_MISSES),
    // real dTLB miss events
    std::make_pair("dTLB-load-misses",          EVENT_DTLB_MISSES),
    std::make_pair("dTLB-store-misses",         EVENT_DTLB_MISSES),
    std::make_pair("dTLB-prefetch-misses",      EVENT_DTLB_MISSES),
  };

  size_t numEle = sizeof idToType / sizeof (*idToType);
  for (size_t i = 0; i < numEle; i++) {
    if (!strcmp(idToType[i].first, eventId)) return idToType[i].second;
  }

  return NUM_EVENT_TYPES;
}

const char* validArguments[] = {
  "cycles",
  "branch-misses",
  "L1-icache-misses",
  "L1-dcache-misses",
  "cache-misses",
  "iTLB-misses",
  "dTLB-misses",
  "prof-counters"
};

PerfEventType commandLineArgumentToEventType(const char* argument) {
  size_t numEle = sizeof validArguments / sizeof (*validArguments);
  always_assert(numEle == NUM_EVENT_TYPES);

  for (size_t i = 0; i < numEle; i++) {
    if (!strcmp(validArguments[i], argument)) return (PerfEventType)i;
  }

  return NUM_EVENT_TYPES;
}

const char* eventTypeToCommandLineArgument(PerfEventType eventType) {
  always_assert(eventType != NUM_EVENT_TYPES);
  return validArguments[eventType];
}

// StackTraceTree

struct StackTraceTree::Node {
  typedef std::unordered_map<string, Node*> StrToPNode;

  uint64_t inclusiveCount;
  uint64_t selfCount; // samples that happen precisely at this node
  StrToPNode children;

  Node() : inclusiveCount(0), selfCount(0) {};

  ~Node() {
    Node::StrToPNode::iterator it;

    for (it = children.begin(); it != children.end(); it++) {
      delete(it->second);
    }
  }

private:
  // These are private so that we don't accidentally copy and later
  // double-delete one of of the pointers in children.
  Node(const Node& other);
  Node& operator=(Node other);
};


StackTraceTree::StackTraceTree() : root(new Node()) {}

void StackTraceTree::insert(const vector<string>& trace, uint64_t count) {
  if (trace.empty()) return;
  root.get()->inclusiveCount += count;

  Node* curr = root.get();
  for (size_t i = 0; i < trace.size(); i++) {
    Node::StrToPNode::const_iterator it = curr->children.find(trace[i]);

    if (it == curr->children.end()) {
      curr->children.insert(std::pair<string, Node*>(trace[i], new Node()));
    }

    curr = curr->children[trace[i]];
    curr->inclusiveCount += count;
  }

  curr->selfCount += count;
}

void StackTraceTree::printTop(double minPercentage,
                              uint64_t eventCount) const {

  always_assert(eventCount);
  uint64_t helpersCount = root.get()->inclusiveCount;

  printf("+ samples in helpers: %" PRIu64 " (%5.2lf%%)\n",
         helpersCount,
         helpersCount * 100.0 / eventCount );

  vector<uint32_t> activeColumns;
  printNode(root.get(), minPercentage, 1, activeColumns, eventCount);
}

void printSeparators(uint32_t n, vector<uint32_t>& activeColumns) {
  if (!n) return;
  always_assert(!activeColumns.empty());

  uint32_t cc = 0;
  for (uint32_t i = 1; i <= n; i++) {
    char c = ' ';
    if (cc < activeColumns.size() && activeColumns[cc] == i) {
      c = '|';
      cc++;
    }
    printf("%c  ", c);
  }
}

void printGuides(uint32_t depth, vector<uint32_t>& activeColumns) {
  printSeparators(depth, activeColumns);
  printf("\n");
  always_assert(depth);
  printSeparators(depth - 1, activeColumns);
  printf("+--+ ");
}

void StackTraceTree::printNode(const Node* node,
                               double minPercentage,
                               uint32_t depth,
                               vector<uint32_t>& activeColumns,
                               uint64_t eventCount) const {
  always_assert(eventCount);
  std::vector<std::pair<uint64_t, string> > activeChildren;

  Node::StrToPNode::const_iterator it;
  for (it = node->children.begin(); it != node->children.end(); it++) {
    if (it->second->inclusiveCount * 100.0 >= minPercentage * eventCount) {
      activeChildren.push_back(make_pair(it->second->inclusiveCount,
                                         it->first));
    }
  }

  std::sort(activeChildren.rbegin(), activeChildren.rend());
  if (!activeChildren.empty()) activeColumns.push_back(depth);

  for (size_t i = 0; i < activeChildren.size(); i++) {

    it = node->children.find(activeChildren[i].second);
    always_assert(it != node->children.end());
    const Node* next = it->second;

    printGuides(depth, activeColumns);
    if (i + 1 == activeChildren.size()) activeColumns.pop_back();

    printf("%" PRIu64 " (%.2lf%%) self: %" PRIu64 " (%.2lf%%) %s\n",
           next->inclusiveCount,
           next->inclusiveCount * 100.0 / eventCount,
           next->selfCount,
           next->selfCount * 100.0 / eventCount,
           activeChildren[i].second.c_str());

    printNode(next,
              minPercentage,
              depth + 1,
              activeColumns,
              eventCount);
  }
}

void StackTraceTree::aggregate(const Node* from, Node *to) {
  always_assert(from != nullptr && to != nullptr);

  to->inclusiveCount += from->inclusiveCount;
  to->selfCount += from->selfCount;

  Node::StrToPNode::const_iterator itFrom, itTo;

  for (itFrom = from->children.begin();
       itFrom != from->children.end();
       itFrom++) {

    itTo = to->children.find(itFrom->first);
    if (itTo == to->children.end()) {
      to->children.insert({itFrom->first, new Node()});
    }

    aggregate(itFrom->second, to->children[itFrom->first]);
  }
}

void StackTraceTree::aggregateTree(const StackTraceTree& other) {
  aggregate(other.root.get(), root.get());
}

} }
