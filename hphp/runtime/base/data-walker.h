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
#ifndef incl_HPHP_DATA_WALKER_H_
#define incl_HPHP_DATA_WALKER_H_

#include "hphp/runtime/base/req-hash-map.h"
#include "hphp/runtime/base/req-hash-set.h"
#include "hphp/util/functional.h" // for pointer_hash

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct HeapObject;
struct ArrayData;
struct ObjectData;
struct TypedValue;

//////////////////////////////////////////////////////////////////////

/*
 * Walk an object or array and find characteristics of the data graph.
 * Clients set up the walker to look for certain characteristics and call
 * traverseData().
 *
 * Used by APC to make proper decisions about the format of the data to save.
 */
struct DataWalker {
  /*
   * Directive for the DataWalker. Define what to look for.
   */
  enum class LookupFeature {
    Default                  = 0x0,
    DetectSerializable       = 0x1,
    HasObjectOrResource      = 0x2
  };

  /*
   * The set of features found by the DataWalker according to what specified
   * via LookupFeature.
   */
  struct DataFeature {
    DataFeature()
      : isCircular(false)
      , hasSerializable(false)
      , hasObjectOrResource(false) {
    }

    // whether the data graph is not a tree in a user-visible way (either
    // circular or DAG of types with reference semantics like objects).
    unsigned isCircular : 1;
    // whether the data graph contains serializable objects
    unsigned hasSerializable : 1;
    // whether the data graph contains any object or resource
    unsigned hasObjectOrResource : 1;
  };

  using PointerSet = req::fast_set<const HeapObject*,
                                   pointer_hash<const HeapObject>>;

  using PointerMap = req::fast_map<HeapObject*, HeapObject*,
                                   pointer_hash<HeapObject>>;

public:
  /*
   * Sets up a DataWalker to analyze an object or array.
   */
  explicit DataWalker(LookupFeature features)
    : m_features(features)
  {}

  DataFeature traverseData(ObjectData* data) const {
    // keep track of visited nodes in an array or object graph
    PointerSet visited;
    DataFeature features;
    traverseData(data, features, visited);
    return features;
  }

  DataFeature traverseData(ArrayData* data,
                           PointerMap* seenArrs = nullptr) const {
    // keep track of visited nodes in an array or object graph
    PointerSet visited;
    DataFeature features;
    traverseData(data, features, visited, seenArrs);
    return features;
  }

private:
  void traverseData(ArrayData* data,
                    DataFeature& features,
                    PointerSet& visited,
                    PointerMap* seenArrs = nullptr) const;
  void traverseData(ObjectData* data,
                    DataFeature& features,
                    PointerSet& visited) const;

  bool visitTypedValue(TypedValue rval,
                       DataFeature& features,
                       PointerSet& visited,
                       PointerMap* seenArrs = nullptr) const;
  bool markVisited(HeapObject* ptr,
                   DataFeature& features,
                   PointerSet& visited) const;
  void objectFeature(ObjectData* pobj, DataFeature&) const;

  bool canStopWalk(DataFeature& features) const;

private:
  // the set of feature to analyze for this walker
  LookupFeature m_features;
};

inline DataWalker::LookupFeature operator|(
    DataWalker::LookupFeature a,
    DataWalker::LookupFeature b) {
  return DataWalker::LookupFeature(
      static_cast<int>(a) | static_cast<int>(b));
}

inline bool operator&(
    DataWalker::LookupFeature a,
    DataWalker::LookupFeature b) {
  return static_cast<int>(a) & static_cast<int>(b);
}

inline DataWalker::LookupFeature operator~(DataWalker::LookupFeature f) {
  return DataWalker::LookupFeature(~static_cast<int>(f));
}

//////////////////////////////////////////////////////////////////////

}

#endif
