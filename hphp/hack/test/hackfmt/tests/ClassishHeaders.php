<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class ClassishHeaders extends ClassishHeadersBase implements IClassishHeaders, IHeaders, IClassish, IAbstract {
}

abstract class ClassishHeadersWithAnExtremelyLongNameThatMakesNoSense extends ClassishHeadersBase implements IClassishHeaders, IHeaders, IClassish, IAbstract {
}

abstract class ClassishHeadersWithAnExtremelyLongNameThatMakesNoSenseBase extends ClassishHeadersBase implements IClassishHeaders, IHeaders, IClassish, IAbstract, IPutsTheExtendsListOverLineLength {
}

interface IClassishHeaders extends IHeaders, IClassish, IInterface, IPutsThisOverLineLengths {
}

interface IClassishHeaders extends IHeaders, IClassish, IInterface, IPutsThisOverLineLength, IPutsTheExtendsListOverLineLength {
}
