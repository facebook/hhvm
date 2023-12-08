<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

interface I<+T> { }

class D implements I<arraykey> { }
class C extends D implements I<string> { }

interface J<+T> { }

class E implements J<arraykey> { }
// Legal
class F extends E implements J<string> { }

// Illegal cos redundant
class G extends E implements J<mixed> { }
// Illegal cos unrelated
class H extends E implements J<float> { }

// Indirect variations on above
interface K extends J<mixed> { }
interface L extends J<float> { }

class GG extends E implements K { }
class HH extends E implements L { }
