# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

proxygen_add_test(TARGET AcceptorTest DEPENDS proxygen testmain)
proxygen_add_test(TARGET RequestWorkerThreadTest DEPENDS proxygen testmain)
