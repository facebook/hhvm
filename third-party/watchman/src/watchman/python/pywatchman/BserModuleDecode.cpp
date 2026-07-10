/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <algorithm>
#include <cstdint>
#include <vector>

extern "C" PyObject* PyInit_bser(void);

namespace {

constexpr uint8_t kBserArray = 0x00;
constexpr uint8_t kBserObject = 0x01;
constexpr uint8_t kBserByteString = 0x02;
constexpr uint8_t kBserInt8 = 0x03;
constexpr uint8_t kBserInt32 = 0x05;
constexpr uint8_t kBserInt64 = 0x06;
constexpr uint8_t kBserTrue = 0x08;
constexpr uint8_t kBserFalse = 0x09;
constexpr uint8_t kBserNull = 0x0a;
constexpr uint8_t kBserTemplate = 0x0b;
constexpr uint8_t kBserSkip = 0x0c;
constexpr size_t kMaxPayloadSize = 64 * 1024;

PyObject* gBserModule = nullptr;

uint32_t readU32(const uint8_t* data, size_t size, size_t offset) {
  uint32_t value = 0;
  for (size_t i = 0; i < sizeof(value) && offset + i < size; ++i) {
    value |= static_cast<uint32_t>(data[offset + i]) << (8 * i);
  }
  return value;
}

uint64_t readU64(const uint8_t* data, size_t size, size_t offset) {
  uint64_t value = 0;
  for (size_t i = 0; i < sizeof(value) && offset + i < size; ++i) {
    value |= static_cast<uint64_t>(data[offset + i]) << (8 * i);
  }
  return value;
}

void appendU32(std::vector<uint8_t>& out, uint32_t value) {
  for (size_t i = 0; i < sizeof(value); ++i) {
    out.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xff));
  }
}

void appendI8(std::vector<uint8_t>& out, uint8_t value) {
  out.push_back(kBserInt8);
  out.push_back(value);
}

void appendI32(std::vector<uint8_t>& out, uint32_t value) {
  out.push_back(kBserInt32);
  appendU32(out, value);
}

void appendI64(std::vector<uint8_t>& out, uint64_t value) {
  out.push_back(kBserInt64);
  for (size_t i = 0; i < sizeof(value); ++i) {
    out.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xff));
  }
}

void appendByteString(
    std::vector<uint8_t>& out,
    const uint8_t* data,
    size_t size) {
  out.push_back(kBserByteString);
  appendI32(out, static_cast<uint32_t>(size));
  out.insert(out.end(), data, data + size);
}

std::vector<uint8_t> makeStructuredValue(const uint8_t* data, size_t size) {
  if (size == 0) {
    return {kBserNull};
  }

  std::vector<uint8_t> out;
  const uint8_t mode = data[0] % 6;
  const uint8_t* payload = data + 1;
  const size_t payloadSize = size - 1;

  switch (mode) {
    case 0:
      out.push_back((data[0] & 0x40) ? kBserTrue : kBserFalse);
      break;
    case 1:
      appendI64(out, readU32(data, size, 1));
      break;
    case 2: {
      const size_t len = std::min(payloadSize, size_t{128});
      appendByteString(out, payload, len);
      break;
    }
    case 3: {
      const uint8_t count =
          static_cast<uint8_t>(std::min(payloadSize, size_t{16}));
      out.push_back(kBserArray);
      appendI8(out, count);
      for (uint8_t i = 0; i < count; ++i) {
        out.push_back((payload[i] & 1) ? kBserTrue : kBserFalse);
      }
      break;
    }
    case 4: {
      const uint8_t count =
          static_cast<uint8_t>(std::min(payloadSize / 2, size_t{8}));
      out.push_back(kBserObject);
      appendI8(out, count);
      for (uint8_t i = 0; i < count; ++i) {
        const uint8_t key = static_cast<uint8_t>('a' + (payload[2 * i] % 26));
        appendByteString(out, &key, 1);
        appendI8(out, payload[2 * i + 1]);
      }
      break;
    }
    case 5: {
      const uint8_t count =
          static_cast<uint8_t>(std::min(payloadSize, size_t{16}));
      const uint8_t key = 'k';
      out.push_back(kBserTemplate);
      out.push_back(kBserArray);
      appendI8(out, 1);
      appendByteString(out, &key, 1);
      appendI8(out, count);
      for (uint8_t i = 0; i < count; ++i) {
        if (payload[i] & 1) {
          out.push_back(kBserSkip);
        } else {
          appendI8(out, payload[i]);
        }
      }
      break;
    }
  }

  return out;
}

std::vector<uint8_t> makePdu(
    uint8_t version,
    uint32_t capabilities,
    uint32_t declaredLength,
    const std::vector<uint8_t>& payload) {
  std::vector<uint8_t> out;
  out.reserve(payload.size() + 11);
  out.push_back(0);
  out.push_back(version);
  if (version == 2) {
    appendU32(out, capabilities);
  }
  appendI32(out, declaredLength);
  out.insert(out.end(), payload.begin(), payload.end());
  return out;
}

std::vector<uint8_t> makePduWithI64Length(
    uint8_t version,
    uint32_t capabilities,
    uint64_t declaredLength,
    const std::vector<uint8_t>& payload) {
  std::vector<uint8_t> out;
  out.reserve(payload.size() + 15);
  out.push_back(0);
  out.push_back(version);
  if (version == 2) {
    appendU32(out, capabilities);
  }
  appendI64(out, declaredLength);
  out.insert(out.end(), payload.begin(), payload.end());
  return out;
}

void callBserMethod(const char* method, const std::vector<uint8_t>& input) {
  if (gBserModule == nullptr) {
    return;
  }

  PyObject* bytes = PyBytes_FromStringAndSize(
      reinterpret_cast<const char*>(input.data()),
      static_cast<Py_ssize_t>(input.size()));
  if (bytes == nullptr) {
    PyErr_Clear();
    return;
  }

  PyObject* result = PyObject_CallMethod(gBserModule, method, "O", bytes);
  Py_DECREF(bytes);
  if (result == nullptr) {
    PyErr_Clear();
    return;
  }
  Py_DECREF(result);
}

void exerciseBserModule(const std::vector<uint8_t>& input) {
  callBserMethod("pdu_info", input);
  callBserMethod("pdu_len", input);
  callBserMethod("loads", input);
}

} // namespace

extern "C" int LionheadFuzzerCustomInitialize(int*, char***) {
  Py_InitializeEx(0);
  gBserModule = PyInit_bser();
  if (gBserModule == nullptr) {
    PyErr_Clear();
  }
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  const size_t payloadSize = std::min(size, kMaxPayloadSize);
  std::vector<uint8_t> raw;
  if (payloadSize > 0) {
    raw.assign(data, data + payloadSize);
  }
  exerciseBserModule(raw);

  std::vector<uint8_t> structured = makeStructuredValue(data, payloadSize);
  exerciseBserModule(
      makePdu(1, 0, static_cast<uint32_t>(structured.size()), structured));
  exerciseBserModule(makePdu(
      2,
      readU32(data, payloadSize, 4),
      static_cast<uint32_t>(structured.size()),
      structured));

  const uint32_t declaredLength = readU32(data, payloadSize, 0);
  exerciseBserModule(makePdu(1, 0, declaredLength, raw));
  exerciseBserModule(
      makePdu(2, readU32(data, payloadSize, 4), declaredLength, raw));

  uint64_t declaredLength64 = readU64(data, payloadSize, 0);
  if (payloadSize > 0 && (data[0] & 0x80) != 0) {
    declaredLength64 |= uint64_t{1} << 63;
  }
  exerciseBserModule(makePduWithI64Length(1, 0, declaredLength64, raw));
  exerciseBserModule(makePduWithI64Length(
      2, readU32(data, payloadSize, 8), declaredLength64, raw));

  return 0;
}
