<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I<T> { }
interface K<T> { }
interface L<T> { }

interface J extends I<string>, K<float>, I<int>, I<bool>, L<int>, K<string> { }
