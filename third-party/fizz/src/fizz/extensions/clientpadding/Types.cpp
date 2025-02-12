/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/extensions/clientpadding/Types.h>

namespace fizz {
template <>
Extension encodeExtension(const extensions::Padding& padding) {
  constexpr size_t kMinPaddingBytes =
      sizeof(ExtensionType) + sizeof(padding.total_bytes);

  Extension ext;
  ext.extension_type = ExtensionType::padding;
  size_t paddingBufLen = padding.total_bytes < kMinPaddingBytes
      ? 0
      : padding.total_bytes - kMinPaddingBytes;
  ext.extension_data = folly::IOBuf::create(paddingBufLen);
  std::memset(ext.extension_data->writableData(), 0, paddingBufLen);
  ext.extension_data->append(paddingBufLen);
  return ext;
}

} // namespace fizz
