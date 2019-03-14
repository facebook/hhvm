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
#include "hphp/tools/tc-print/perf-events.h"

#include <unordered_map>
#include <vector>

#include "hphp/tools/tc-print/tc-print.h"

namespace HPHP { namespace jit {

using std::string;

// Predefined PerfEvent mappings.
std::unordered_map<std::string, PerfEventType> idToType = {
  {"cycles", EVENT_CYCLES},
  {"branch-misses", EVENT_BRANCH_MISSES},
  {"L1-icache-misses", EVENT_ICACHE_MISSES},
  {"L1-dcache-misses", EVENT_DCACHE_MISSES},
  {"cache-misses", EVENT_LLC_MISSES},
  {"LLC-store-misses", EVENT_LLC_STORE_MISSES},
  {"iTLB-misses", EVENT_ITLB_MISSES},
  {"dTLB-misses", EVENT_DTLB_MISSES}
};

std::vector<std::string> smallCaptions = {
  "cy", "bm", "ic", "dc", "lc", "sm", "it", "dt"
};

bool usePredefinedTypes = true;
size_t numEventTypes = NUM_PREDEFINED_EVENT_TYPES;

size_t getFirstEventType() {
  return usePredefinedTypes ? 0 : NUM_PREDEFINED_EVENT_TYPES;
}

size_t getNumEventTypes() {
  return usePredefinedTypes ? NUM_PREDEFINED_EVENT_TYPES : numEventTypes;
}

void addEventType(std::string  eventId) {
  usePredefinedTypes = false;

  auto it = idToType.find(eventId);
  if (it == idToType.end()) {
    idToType.insert({eventId, static_cast<PerfEventType>(numEventTypes)});
  } else {
    it->second = static_cast<PerfEventType>(numEventTypes);
  }

  char caption[] = "xx";
  sprintf(caption, "u%c",
          static_cast<char>(numEventTypes - getFirstEventType()) + 'A');
  smallCaptions.push_back(std::string(caption));

  numEventTypes++;
}

PerfEventType perfScriptOutputToEventType(std::string eventId) {
  auto it = idToType.find(eventId);
  if (it == idToType.end()) {
    return EVENT_NULL;
  }
  return it->second;
}

PerfEventType commandLineArgumentToEventType(const char* argument) {
  return perfScriptOutputToEventType(std::string(argument));
}

const char* eventTypeToCommandLineArgument(PerfEventType eventType) {
  for (auto it = idToType.begin(); it != idToType.end(); ++it) {
    if (it->second == eventType) {
      return it->first.c_str();
    }
  }
  always_assert(false);
  return "Error: Event Type not supported.";
}

const char* eventTypeToSmallCaption(PerfEventType eventType) {
  return smallCaptions[eventType].c_str();
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
