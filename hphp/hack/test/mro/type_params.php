<?hh

interface Ico<+T> {}


interface I2 extends Ico<mixed> {}

interface I3 extends I2, Ico<num> {}

interface I4 extends I3, Ico<int> {}
