# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

import os.path

if "WATCHMAN_INTEGRATION_HELPERS" in os.environ:
    HELPER_ROOT = os.path.join(os.environ["WATCHMAN_INTEGRATION_HELPERS"])
else:
    WATCHMAN_SRC_DIR: str = os.environ.get("WATCHMAN_SRC_DIR", os.getcwd())
    HELPER_ROOT = os.path.join(WATCHMAN_SRC_DIR, "integration")
