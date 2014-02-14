#ifndef incl_HPHP_JSON_CHANNELED_COMPRESSOR_H_
#define incl_HPHP_JSON_CHANNELED_COMPRESSOR_H_

#include <vector>
#include <map>

#include "folly/dynamic.h"
#include "folly/FBString.h"
#include "folly/FBVector.h"
#include "folly/io/IOBufQueue.h"

#include "hphp/util/gen-cpp/channeled_json_compressor_types.h"



namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
A class that compresses json files according to channels
*/
class ChanneledJsonCompressor {

public:
  ChanneledJsonCompressor();
  ~ChanneledJsonCompressor();

  // Processes the json input buffer
  void processJson(const char* data, int length);

  // Returns a buffer containing the compressed channels and their length
  void finalize(folly::IOBufQueue& outBufQueue);

private:

  // write an object to the underlying compressors
  void write(const folly::dynamic& obj);

  // Write a variable length integer into one of the channels
  void writeVarInt(Channel channel, int64_t value);

  // write a type (The type is not Type because there are intermediate values)
  void writeType(uint8_t jsonType);

  // write a map
  void writeMap(const folly::dynamic& mapObj);

  // write an array
  void writeArray(const folly::dynamic& arrayObj);

  // write a string
  void writeString(const folly::dynamic& stringObj);

  // write a double
  void writeDouble(const folly::dynamic& doubleObj);

private:
  // output channels (implemented as strings).
  // see this discussion of stringstream here: https://fburl.com/16088028
  folly::fbvector<folly::fbstring> channels_;

  // memoising strings
  std::map<folly::fbstring, int> memoStrings_;
  uint32_t memoStringsNext_;

  //memoising keys
  std::map<folly::fbstring, int> memoKeys_;
  uint32_t memoKeysNext_;

};

} // namesapce HPHP

#endif // incl_HPHP_JSON_CHANNELED_COMPRESSOR_H_
