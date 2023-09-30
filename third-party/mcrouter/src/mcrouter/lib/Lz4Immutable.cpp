/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Lz4Immutable.h"

#include <folly/Format.h>
#include <folly/lang/Bits.h>

namespace facebook {
namespace memcache {

namespace {

constexpr size_t kHashUnit = sizeof(size_t);
constexpr size_t kMaxDictionarySize = 64 * 1024;

// Used for hasing / fill hash table
constexpr uint32_t kHashMask = (1 << kHashLog) - 1;
constexpr uint64_t kPrime5Bytes = 889523592379ULL;

// Min match size.
constexpr size_t kMinMatch = 4;
// Size of the copies.
constexpr size_t kCopyLength = 8;
// Size of the last literals.
constexpr size_t kLastLiterals = 5;
// We will look for matches until there is just this number of bytes remaining.
constexpr size_t kMatchFindLimit = kCopyLength + kMinMatch;
// Minimum input size this algorithm accepts.
constexpr size_t kMinInputSize = kMatchFindLimit + 1;
// Maximum input size this algorithm accepts.
constexpr uint32_t kMaxInputSize = 0x7E000000;

constexpr size_t kSkipTrigger = 6;
constexpr size_t kStepSize = sizeof(uint64_t);

// Output formating - Ml stands for Match Length.
constexpr size_t kMlBits = 4;
constexpr size_t kMlMask = (1U << kMlBits) - 1;
constexpr size_t kRunBits = 8 - kMlBits;
constexpr size_t kRunMask = (1U << kRunBits) - 1;

uint32_t hashSequence(size_t sequence) {
  return ((sequence * kPrime5Bytes) >> (40 - kHashLog)) & kHashMask;
}

uint32_t hashPosition(const IovecCursor& cursor) {
  return hashSequence(cursor.peek<uint64_t>());
}

uint32_t getPositionOnHash(const Hashtable& table, uint32_t hash) {
  return table[hash];
}

void putPosition(Hashtable& table, const IovecCursor& cursor) {
  uint32_t h = hashPosition(cursor);
  table[h] = cursor.tell();
}

struct iovec getDictionaryIovec(const Lz4ImmutableState& state) noexcept {
  struct iovec iov;
  state.dictionary->fillIov(&iov, 1);
  return iov;
}

void checkInputSize(size_t inputLength) {
  if (inputLength > kMaxInputSize) {
    throw std::invalid_argument(folly::sformat(
        "Data too large to compress. Size: {}. Max size allowed: {}",
        inputLength,
        kMaxInputSize));
  }
}

Lz4ImmutableState loadDictionary(std::unique_ptr<folly::IOBuf> dictionary) {
  if (!dictionary) {
    throw std::invalid_argument("Dictionary cannot be nullptr.");
  }
  if (dictionary->isChained()) {
    throw std::invalid_argument("Dictionary has to be coalesced.");
  }

  size_t dicSize = dictionary->length();
  if (dicSize < kHashUnit) {
    throw std::invalid_argument(
        folly::sformat("Dictionary cannot be smaller than {}.", kHashUnit));
  }
  if (dicSize > kMaxDictionarySize) {
    throw std::invalid_argument(folly::sformat(
        "Dictionary cannot be larger than {}.", kMaxDictionarySize));
  }

  Lz4ImmutableState state;
  state.dictionary = std::move(dictionary);
  state.table.fill(0);

  struct iovec dicIov = getDictionaryIovec(state);
  IovecCursor dicCursor(&dicIov, 1);
  while (dicCursor.tell() <= dicSize - kHashUnit) {
    putPosition(state.table, dicCursor);
    dicCursor.advance(3);
  }

  return state;
}

/**
 * A customized version of std::memcpy that works with chained IOBufs.
 *
 * Note: cursor will point just past its current position + "count"
 */
void safeCopy(uint8_t* dest, IovecCursor& source, size_t count) {
  int64_t left = count;
  uint64_t src;
  do {
    size_t toWrite = std::min((int64_t)8, left);
    if (FOLLY_LIKELY(toWrite == sizeof(uint64_t))) {
      src = source.peek<uint64_t>();
    } else {
      source.peekInto(reinterpret_cast<uint8_t*>(&src), toWrite);
    }
    std::memcpy(dest, &src, toWrite);
    dest += toWrite;
    source.advance(toWrite);
    left -= toWrite;
  } while (left > 0);
}

/**
 * A customized (faster) version of safeCopy that may overwrite
 * up to 7 bytes more than "count".
 *
 * Note: cursor will point just past it's current position + "count" + up to
 * 7 bytes.
 */
void wildCopy(uint8_t* dest, IovecCursor& cursor, size_t count) {
  const uint8_t* destEnd = dest + count;
  do {
    uint64_t src = cursor.read<uint64_t>();
    std::memcpy(dest, &src, sizeof(uint64_t));
    dest += sizeof(uint64_t);
  } while (dest < destEnd);
}

void writeLE(void* dest, uint16_t val) {
  uint16_t valLE = folly::Endian::little(val);
  std::memcpy(dest, &valLE, sizeof(uint16_t));
}

uint16_t peekLE(const IovecCursor& cursor) {
  return folly::Endian::little(cursor.peek<uint16_t>());
}

/**
 * Given the difference between two number (i.e. num1 ^ num2), return the number
 * of leading bytes that are common in the two numbers.
 *
 * @param diff  Result of XOR between two numbers.
 * @return      The number of bytes that are common in the two numbers.
 */
size_t numCommonBytes(size_t diff) {
#if (defined(__clang__) || ((__GNUC__ * 100 + __GNUC_MINOR__) >= 304))
  if (folly::Endian::order == folly::Endian::Order::LITTLE) {
    // __builtin_ctzll returns the number of trailling zeros in the binary
    // representation of "diff".
    return __builtin_ctzll(diff) >> 3; // we care about bytes (group of 8 bits).
  } else {
    // __builtin_ctzll returns the number of leading zeros in the binary
    // representation of "diff".
    return __builtin_clzll(diff) >> 3; // we care about bytes (group of 8 bits).
  }
#else
#error "Clang or GCC >= 304 required for Immutable Lz4 compression."
#endif
}

/**
 * Calculates the length of a match given a starting point.
 *
 * @param source  Source cursor.
 * @param match   Match cursor.
 * @param limit   Upper limit that points just past where "source" can
 *                go to find a match.
 *
 * @return              The size of the match, in bytes.
 */
size_t
calculateMatchLength(IovecCursor& source, IovecCursor& match, size_t limit) {
  const size_t start = source.tell();

  while (FOLLY_LIKELY(source.tell() < limit - kStepSize - 1)) {
    uint64_t diff = match.peek<uint64_t>() ^ source.peek<uint64_t>();
    if (!diff) {
      source.advance(kStepSize);
      match.advance(kStepSize);
      continue;
    }
    size_t commonBytes = numCommonBytes(diff);
    source.advance(commonBytes);
    match.advance(commonBytes);
    return source.tell() - start;
  }

  if ((source.tell() < limit - 3) &&
      (match.peek<uint32_t>() == source.peek<uint32_t>())) {
    source.advance(sizeof(uint32_t));
    match.advance(sizeof(uint32_t));
  }
  if ((source.tell() < limit - 1) &&
      (match.peek<uint16_t>() == source.peek<uint16_t>())) {
    source.advance(sizeof(uint16_t));
    match.advance(sizeof(uint16_t));
  }
  if ((source.tell() < limit) &&
      (match.peek<uint8_t>() == source.peek<uint8_t>())) {
    source.advance(sizeof(uint8_t));
    match.advance(sizeof(uint8_t));
  }
  return source.tell() - start;
}

} // anonymous namespace

Lz4Immutable::Lz4Immutable(std::unique_ptr<folly::IOBuf> dictionary)
    : state_(loadDictionary(std::move(dictionary))) {}

size_t Lz4Immutable::compressBound(size_t size) const noexcept {
  return size + ((size / 255) + 16);
}

std::unique_ptr<folly::IOBuf> Lz4Immutable::compress(
    const folly::IOBuf& source) const {
  auto iov = source.getIov();
  return compress(iov.data(), iov.size());
}

std::unique_ptr<folly::IOBuf> Lz4Immutable::compress(
    const struct iovec* iov,
    size_t iovcnt) const {
  if (FOLLY_UNLIKELY(iovcnt == 0)) {
    return folly::IOBuf::create(0);
  }

  // Check the max size prior to allocating space.
  IovecCursor source(iov, iovcnt);
  checkInputSize(source.totalLength());

  const size_t maxOutputSize = compressBound(source.totalLength());
  auto destination = folly::IOBuf::create(maxOutputSize);
  const size_t compressedSize =
      compressCommon(source, destination->writableTail(), maxOutputSize);
  destination->append(compressedSize);
  return destination;
}

size_t Lz4Immutable::compressInto(
    const struct iovec* iov,
    size_t iovcnt,
    void* dest,
    size_t destSize) const {
  if (FOLLY_UNLIKELY(iovcnt == 0)) {
    return 0;
  }

  IovecCursor source(iov, iovcnt);
  checkInputSize(source.totalLength());

  if (FOLLY_UNLIKELY(destSize < compressBound(source.totalLength()))) {
    throw std::invalid_argument(folly::sformat(
        "Destination too small. Size: {}. Required: {}",
        destSize,
        compressBound(source.totalLength())));
  }

  return compressCommon(source, static_cast<uint8_t*>(dest), destSize);
}

size_t Lz4Immutable::compressCommon(
    IovecCursor source,
    uint8_t* output,
    size_t maxOutputSize) const {
  assert(source.totalLength() <= kMaxInputSize && "check max size first");
  assert(
      maxOutputSize >= compressBound(source.totalLength()) &&
      "check available space first");

  // Creates a match cursor - a cursor that will keep track of matches
  // found in the dictionary.
  struct iovec dicIov = getDictionaryIovec(state_);
  const IovecCursor dicCursor(&dicIov, 1);

  // The difference between the dictionary size and the max we can look back
  // to find a match (64KB).
  // It is used to see if a match is valid to be used (it has to
  // be, at most, 64KB "behind" the data we are compresing right now).
  const size_t dictionaryDiff = kMaxDictionarySize - dicCursor.totalLength();

  // Upper limit of where we can look for a match.
  const size_t matchFindLimit = source.totalLength() - kMatchFindLimit;
  // Upper limit of where a match can go.
  const size_t matchLimit = source.totalLength() - kLastLiterals;

  // Lower and upper limit to where the output buffer can go.
  const uint8_t* const outputStart = output;
  const uint8_t* const outputLimit = output + maxOutputSize;
  (void)outputLimit;

  // Controls the compression main loop.
  bool running = true;

  // Next position (0..sourceSize] in source that was not
  // yet written to destination buffer.
  IovecCursor anchorCursor = source;

  // Cursor that points to current match.
  IovecCursor match = dicCursor;

  if (source.totalLength() < kMinInputSize) {
    // Not enough data to compress. Don't even enter the compress loop,
    // skip directly to the part that encodes the last literals.
    running = false;
  } else {
    // Skip first byte.
    source.advance(1);
  }

  // Main loop
  while (running) {
    // LZ4 token
    uint8_t* token;

    // Find a match
    {
      size_t step = 0;
      size_t searchMatchNumBytes = 1 << kSkipTrigger;

      do {
        // Advance cursor and calculate next step.
        source.advance(step);
        step = (searchMatchNumBytes++ >> kSkipTrigger);

        // Hash current position
        uint32_t hash = hashPosition(source);

        // Verify if the current position in the source buffer
        // can still be compressed.
        if (FOLLY_UNLIKELY(source.tell() + step > matchFindLimit) ||
            FOLLY_UNLIKELY(source.tell() > kMaxDictionarySize)) {
          running = false;
          break;
        }

        uint32_t matchPos = getPositionOnHash(state_.table, hash);
        match.seek(matchPos);
      } while (((match.tell() + dictionaryDiff) <= source.tell()) ||
               (match.peek<uint32_t>() != source.peek<uint32_t>()));

      if (!running) {
        break;
      }
    }

    // Catch up - try to expand the match backwards.
    while (source.tell() > anchorCursor.tell() && match.tell() > 0) {
      source.retreat(1);
      match.retreat(1);
      if (FOLLY_LIKELY(source.peek<uint8_t>() != match.peek<uint8_t>())) {
        source.advance(1);
        match.advance(1);
        break;
      }
    }

    // Write literal
    {
      size_t literalLen = source.tell() - anchorCursor.tell();
      token = output++;

      // Check output limit
      assert(
          output + literalLen + (2 + 1 + kLastLiterals) + (literalLen / 255) <=
          outputLimit);

      // Encode literal length
      if (literalLen >= kRunMask) {
        int len = static_cast<int>(literalLen - kRunMask);
        *token = (kRunMask << kMlBits);
        for (; len >= 255; len -= 255) {
          *output++ = 255;
        }
        *output++ = static_cast<uint8_t>(len);
      } else {
        *token = static_cast<uint8_t>(literalLen << kMlBits);
      }

      // Copy literals to output buffer.
      wildCopy(output, anchorCursor, literalLen);
      output += literalLen;
    }

    // Encode offset
    uint16_t offset = dicCursor.totalLength() - match.tell() + source.tell();
    writeLE(output, static_cast<uint16_t>(offset));
    output += 2;

    // Encode matchLength
    {
      // we cannot go past the dictionary
      size_t posLimit =
          source.tell() + (dicCursor.totalLength() - match.tell());
      // Nor go past the source buffer
      posLimit = std::min(posLimit, matchLimit);

      source.advance(kMinMatch);
      match.advance(kMinMatch);
      size_t matchLen = calculateMatchLength(source, match, posLimit);

      assert(output + (1 + kLastLiterals) + (matchLen >> 8) <= outputLimit);

      // Write match length.
      if (matchLen >= kMlMask) {
        *token += kMlMask;
        matchLen -= kMlMask;
        for (; matchLen >= 510; matchLen -= 510) {
          *output++ = 255;
          *output++ = 255;
        }
        if (matchLen >= 255) {
          matchLen -= 255;
          *output++ = 255;
        }
        *output++ = static_cast<uint8_t>(matchLen);
      } else {
        *token += static_cast<uint8_t>(matchLen);
      }
    }

    // Update anchor
    anchorCursor.seek(source.tell());

    // Test end of chunk
    if (source.tell() > matchFindLimit) {
      break;
    }
  }

  // Encode last literals
  {
    size_t lastRun = source.totalLength() - anchorCursor.tell();

    assert(
        (output - outputStart) + lastRun + 1 +
            ((lastRun + 255 - kRunMask) / 255) <=
        maxOutputSize);

    if (lastRun >= kRunMask) {
      size_t accumulator = lastRun - kRunMask;
      *output++ = kRunMask << kMlBits;
      for (; accumulator >= 255; accumulator -= 255) {
        *output++ = 255;
      }
      *output++ = static_cast<uint8_t>(accumulator);
    } else {
      *output++ = static_cast<uint8_t>(lastRun << kMlBits);
    }
    safeCopy(output, anchorCursor, lastRun);
    output += lastRun;
  }

  return output - outputStart;
}

std::unique_ptr<folly::IOBuf> Lz4Immutable::decompress(
    const folly::IOBuf& source,
    size_t uncompressedSize) const noexcept {
  auto iov = source.getIov();
  return decompress(iov.data(), iov.size(), uncompressedSize);
}

std::unique_ptr<folly::IOBuf> Lz4Immutable::decompress(
    const struct iovec* iov,
    size_t iovcnt,
    size_t uncompressedSize) const noexcept {
  if (FOLLY_UNLIKELY(uncompressedSize == 0)) {
    return folly::IOBuf::create(0);
  }

  // Creates a match cursor - a cursor that will keep track of matches
  // found in the dictionary.
  struct iovec dicIov = getDictionaryIovec(state_);
  const IovecCursor dicCursor(&dicIov, 1);

  // Destination (uncompressed) buffer.
  auto destination = folly::IOBuf::create(uncompressedSize);
  // Pointer to where the next uncompressed position should be written.
  uint8_t* output = destination->writableTail();
  // Lower and upper limit to where the output buffer can go.
  const uint8_t* outputStart = output;
  const uint8_t* outputLimit = output + uncompressedSize;
  (void)outputLimit;

  IovecCursor source(iov, iovcnt);
  IovecCursor match = dicCursor;

  // Main loop
  while (true) {
    // LZ4 token
    size_t token = source.read<uint8_t>();

    // Get literal length
    size_t literalLength = token >> kMlBits;
    if (literalLength == kRunMask) {
      size_t s;
      do {
        s = source.read<uint8_t>();
        literalLength += s;
      } while (FOLLY_LIKELY(s == 255));
    }

    // Copy literals
    uint8_t* cpy = output + literalLength;
    if (cpy > outputLimit - kCopyLength) {
      if (cpy != outputLimit) {
        return nullptr;
      }
      safeCopy(output, source, literalLength);
      output += literalLength;
      break; // Necessarily EOF, due to parsing restrictions
    }
    safeCopy(output, source, literalLength);
    output = cpy;

    // Get match offset
    uint16_t offset = peekLE(source);
    size_t matchPos = dicCursor.totalLength() + (output - outputStart) - offset;
    source.advance(2);

    // Get match length
    size_t matchLength = token & kMlMask;
    if (matchLength == kMlMask) {
      size_t s;
      do {
        s = source.read<uint8_t>();
        matchLength += s;
      } while (s == 255);
    }
    matchLength += kMinMatch;

    // Copy match
    match.seek(matchPos);
    safeCopy(output, match, matchLength);
    output += matchLength;
  }

  destination->append(output - outputStart);
  return destination;
}

} // namespace memcache
} // namespace facebook
