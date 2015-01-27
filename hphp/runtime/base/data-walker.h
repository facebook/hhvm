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
#ifndef incl_HPHP_DATA_WALKER_H_
#define incl_HPHP_DATA_WALKER_H_

#include "hphp/util/hash-map-typedefs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct ArrayData;
struct ObjectData;

//////////////////////////////////////////////////////////////////////

/*
 * Walk an object or array and find characteristics of the data graph.
 * Clients set up the walker to look for certain characteristics and call
 * traverseData().
 *
 * Used by APC to make proper decisions about the format of the data to save.
 */
struct DataWalker {
  /**
   * Directive for the DataWalker. Define what to look for.
   */
  enum class LookupFeature {
    Default                  = 0x0,
    DetectSerializable       = 0x1,
    RefCountedReference      = 0x2,
    HasObjectOrResource      = 0x4
  };

  /**
   * The set of features found by the DataWalker according to what specified
   * via LookupFeature.
   */
  struct DataFeature {
    DataFeature()
      : m_circular(false)
      , m_serializable(false)
      , m_hasCollection(false)
      , m_hasRefCountReference(false)
      , m_hasObjectOrResource(false) {
    }

    bool isCircular() const {
      return m_circular;
    }

    bool hasCollection() const {
      return m_hasCollection;
    }

    bool hasSerializableReference() {
      return m_serializable;
    }

    bool hasRefCountReference() const {
      return m_hasRefCountReference;
    }

    bool hasObjectOrResource() const {
      return m_hasObjectOrResource;
    }

  private:
    // whether the data graph contains internal references (it's circular)
    unsigned m_circular : 1;
    // whetehr the data graph contains serialiazble objects
    unsigned m_serializable : 1;
    // whether the data graph contains collections
    unsigned m_hasCollection : 1;
    // whether the data graph contains some ref counted reference
    // (*not* one of: bool, int, double and static string)
    unsigned m_hasRefCountReference : 1;
    // whether the data graph contains any object or resource
    unsigned m_hasObjectOrResource : 1;

    friend class DataWalker;
  };

public:
  /**
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

  DataFeature traverseData(ArrayData* data) const {
    // keep track of visited nodes in an array or object graph
    PointerSet visited;
    DataFeature features;
    traverseData(data, features, visited);
    return features;
  }

private:
  void traverseData(ArrayData* data,
                    DataFeature& features,
                    PointerSet& visited) const;
  void traverseData(ObjectData* data,
                    DataFeature& features,
                    PointerSet& visited) const;

  bool markVisited(void* pvar,
                   DataFeature& features,
                   PointerSet& visited) const;
  void objectFeature(ObjectData* pobj, DataFeature& features) const;

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
