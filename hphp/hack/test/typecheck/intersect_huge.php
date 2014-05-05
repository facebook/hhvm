<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

abstract class TType {
  const STOP   = 0;
  const VOID   = 1;
  const BOOL   = 2;
  const BYTE   = 3;
  const I08    = 3;
  const DOUBLE = 4;
  const I16    = 6;
  const I32    = 8;
  const I64    = 10;
  const STRING = 11;
  const UTF7   = 11;
  const STRUCT = 12;
  const MAP    = 13;
  const SET    = 14;
  const LST    = 15;
  const UTF8   = 16;
  const UTF16  = 17;
  const FLOAT  = 19;
}

class C {
  public string $ivar1;
  public string $ivar2;
  public string $ivar3;
  public int $ivar4;
  public string $ivar5;
  public string $ivar6;
  public string $ivar7;
  public bool $ivar8;
  public ?Ty1 $ivar9;
  public Vector<string> $ivar10;
  public Vector<Ty2> $ivar11;
  public ?Ty2 $ivar12;
  public Vector<string> $ivar13;
  public Map<string, string> $ivar14;
  public int $ivar15;
  public bool $ivar16;
  public Map<string, string> $ivar17;
  public string $ivar18;
  public string $ivar19;
  public int $ivar20;
  public int $ivar21;
  public Vector<string> $ivar22;
  public Map<string, Ty3> $ivar23;
  public ?Ty4 $ivar24;
  public int $ivar25;
  public int $ivar26;
  public Vector<string> $ivar27;
  public Vector<string> $ivar28;
  public string $ivar29;
  public string $ivar30;
  public ?Ty5 $ivar31;
  public Vector<string> $ivar32;
  public Vector<Ty2> $ivar33;
  public int $ivar34;
  public bool $ivar35;
  public Vector<Ty2> $ivar36;
  public Vector<string> $ivar37;
  public string $ivar38;
  public string $ivar39;
  public string $ivar40;
  public string $ivar41;
  public ?Ty6 $ivar42;
  public int $ivar43;
  public ?Ty7 $ivar44;
  public string $ivar45;
  public Map<string, string> $ivar46;
  public Map<string, string> $ivar47;
  public Vector<Ty8> $ivar48;
  public int $ivar49;
  public ?Ty3 $ivar50;
  public Map<int, Ty9> $ivar51;
  public ?Ty10 $ivar52;
  public string $ivar53;

  public function __construct(?Indexish<string, mixed> $vals = null) {
    $this->ivar1 = (string)idx($vals, 'ivar1', '');
    $this->ivar2 = (string)idx($vals, 'ivar2', '');
    $this->ivar3 = (string)idx($vals, 'ivar3', '');
    $this->ivar4 = (int)idx($vals, 'ivar4', 0);
    $this->ivar5 = (string)idx($vals, 'ivar5', '');
    $this->ivar6 = (string)idx($vals, 'ivar6', '');
    $this->ivar7 = (string)idx($vals, 'ivar7', '');
    $this->ivar8 = (bool)idx($vals, 'ivar8', false);
    $this->ivar9 = idx($vals, 'ivar9', null);
    $this->ivar10 = idx($vals, 'ivar10', Vector {});
    $this->ivar11 = idx($vals, 'ivar11', Vector {});
    $this->ivar12 = idx($vals, 'ivar12', null);
    $this->ivar13 = idx($vals, 'ivar13', Vector {});
    $this->ivar14 = idx($vals, 'ivar14', Map {});
    $this->ivar15 = (int)idx($vals, 'ivar15', 0);
    $this->ivar16 = (bool)idx($vals, 'ivar16', false);
    $this->ivar17 = idx($vals, 'ivar17', Map {});
    $this->ivar18 = (string)idx($vals, 'ivar18', '');
    $this->ivar19 = (string)idx($vals, 'ivar19', '');
    $this->ivar20 = (int)idx($vals, 'ivar20', 0);
    $this->ivar21 = (int)idx($vals, 'ivar21', 0);
    $this->ivar22 = idx($vals, 'ivar22', Vector {});
    $this->ivar23 = idx($vals, 'ivar23', Map {});
    $this->ivar24 = idx($vals, 'ivar24', null);
    $this->ivar25 = (int)idx($vals, 'ivar25', 0);
    $this->ivar26 = (int)idx($vals, 'ivar26', 0);
    $this->ivar27 = idx($vals, 'ivar27', Vector {});
    $this->ivar28 = idx($vals, 'ivar28', Vector {});
    $this->ivar29 = (string)idx($vals, 'ivar29', '');
    $this->ivar30 = (string)idx($vals, 'ivar30', '');
    $this->ivar31 = idx($vals, 'ivar31', null);
    $this->ivar32 = idx($vals, 'ivar32', Vector {});
    $this->ivar33 = idx($vals, 'ivar33', Vector {});
    $this->ivar34 = (int)idx($vals, 'ivar34', 0);
    $this->ivar35 = (bool)idx($vals, 'ivar35', false);
    $this->ivar36 = idx($vals, 'ivar36', Vector {});
    $this->ivar37 = idx($vals, 'ivar37', Vector {});
    $this->ivar38 = (string)idx($vals, 'ivar38', '');
    $this->ivar39 = (string)idx($vals, 'ivar39', '');
    $this->ivar40 = (string)idx($vals, 'ivar40', '');
    $this->ivar41 = (string)idx($vals, 'ivar41', '');
    $this->ivar42 = idx($vals, 'ivar42', null);
    $this->ivar43 = (int)idx($vals, 'ivar43', 0);
    $this->ivar44 = idx($vals, 'ivar44', null);
    $this->ivar45 = (string)idx($vals, 'ivar45', '');
    $this->ivar46 = idx($vals, 'ivar46', Map {});
    $this->ivar47 = idx($vals, 'ivar47', Map {});
    $this->ivar48 = idx($vals, 'ivar48', Vector {});
    $this->ivar49 = (int)idx($vals, 'ivar49', 0);
    $this->ivar50 = idx($vals, 'ivar50', null);
    $this->ivar51 = idx($vals, 'ivar51', Map {});
    $this->ivar52 = idx($vals, 'ivar52', null);
    $this->ivar53 = (string)idx($vals, 'ivar53', '');
  }

  public function read(TProtocol $input) : int {
    $xfer = 0;
    $fname = null;
    $ftype = 0;
    $fid = 0;
    $xfer += $input->readStructBegin($fname);
    while (true)
    {
      $xfer += $input->readFieldBegin($fname, $ftype, $fid);
      if ($ftype == TType::STOP) {
        break;
      }
      switch ($fid)
      {
        case 6:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar1);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 7:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar2);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 8:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar3);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 10:
          if ($ftype == TType::I64) {
            $xfer += $input->readI64($this->ivar4);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 11:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar5);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 12:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar6);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 14:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar7);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 15:
          if ($ftype == TType::BOOL) {
            $xfer += $input->readBool($this->ivar8);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 18:
          if ($ftype == TType::STRUCT) {
            $this->ivar9 = new Ty1();
            $xfer += $this->ivar9->read($input);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 20:
          if ($ftype == TType::LST) {
            $_size443 = 0;
            $_val442 = Vector {};
            $_etype446 = 0;
            $xfer += $input->readListBegin($_etype446, $_size443);
            for ($_i447 = 0; $_i447 < $_size443; ++$_i447)
            {
              $elem448 = null;
              $xfer += $input->readString($elem448);
              if ($elem448 !== null) {
                $_val442 []= $elem448;
              }
            }
            $xfer += $input->readListEnd();
            $this->ivar10 = $_val442;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 21:
          if ($ftype == TType::LST) {
            $_size450 = 0;
            $_val449 = Vector {};
            $_etype453 = 0;
            $xfer += $input->readListBegin($_etype453, $_size450);
            for ($_i454 = 0; $_i454 < $_size450; ++$_i454)
            {
              $elem455 = null;
              $elem455 = new Ty2();
              $xfer += $elem455->read($input);
              if ($elem455 !== null) {
                $_val449 []= $elem455;
              }
            }
            $xfer += $input->readListEnd();
            $this->ivar11 = $_val449;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 22:
          if ($ftype == TType::STRUCT) {
            $this->ivar12 = new Ty2();
            $xfer += $this->ivar12->read($input);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 23:
          if ($ftype == TType::LST) {
            $_size457 = 0;
            $_val456 = Vector {};
            $_etype460 = 0;
            $xfer += $input->readListBegin($_etype460, $_size457);
            for ($_i461 = 0; $_i461 < $_size457; ++$_i461)
            {
              $elem462 = null;
              $xfer += $input->readString($elem462);
              if ($elem462 !== null) {
                $_val456 []= $elem462;
              }
            }
            $xfer += $input->readListEnd();
            $this->ivar13 = $_val456;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 26:
          if ($ftype == TType::MAP) {
            $_size464 = 0;
            $_val463 = Map {};
            $_ktype465 = 0;
            $_vtype466 = 0;
            $xfer += $input->readMapBegin($_ktype465, $_vtype466, $_size464);
            for ($_i468 = 0; $_i468 < $_size464; ++$_i468)
            {
              $key469 = '';
              $val470 = '';
              $xfer += $input->readString($key469);
              $xfer += $input->readString($val470);
              if ($key469 !== null && $val470 !== null) {
                $_val463[$key469] = $val470;
              }
            }
            $xfer += $input->readMapEnd();
            $this->ivar14 = $_val463;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 27:
          if ($ftype == TType::I64) {
            $xfer += $input->readI64($this->ivar15);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 32:
          if ($ftype == TType::BOOL) {
            $xfer += $input->readBool($this->ivar16);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 33:
          if ($ftype == TType::MAP) {
            $_size472 = 0;
            $_val471 = Map {};
            $_ktype473 = 0;
            $_vtype474 = 0;
            $xfer += $input->readMapBegin($_ktype473, $_vtype474, $_size472);
            for ($_i476 = 0; $_i476 < $_size472; ++$_i476)
            {
              $key477 = '';
              $val478 = '';
              $xfer += $input->readString($key477);
              $xfer += $input->readString($val478);
              if ($key477 !== null && $val478 !== null) {
                $_val471[$key477] = $val478;
              }
            }
            $xfer += $input->readMapEnd();
            $this->ivar17 = $_val471;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 34:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar18);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 37:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar19);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 38:
          if ($ftype == TType::I64) {
            $xfer += $input->readI64($this->ivar20);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 39:
          if ($ftype == TType::I64) {
            $xfer += $input->readI64($this->ivar21);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 40:
          if ($ftype == TType::LST) {
            $_size480 = 0;
            $_val479 = Vector {};
            $_etype483 = 0;
            $xfer += $input->readListBegin($_etype483, $_size480);
            for ($_i484 = 0; $_i484 < $_size480; ++$_i484)
            {
              $elem485 = null;
              $xfer += $input->readString($elem485);
              if ($elem485 !== null) {
                $_val479 []= $elem485;
              }
            }
            $xfer += $input->readListEnd();
            $this->ivar22 = $_val479;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 41:
          if ($ftype == TType::MAP) {
            $_size487 = 0;
            $_val486 = Map {};
            $_ktype488 = 0;
            $_vtype489 = 0;
            $xfer += $input->readMapBegin($_ktype488, $_vtype489, $_size487);
            for ($_i491 = 0; $_i491 < $_size487; ++$_i491)
            {
              $key492 = '';
              $val493 = new Ty3();
              $xfer += $input->readString($key492);
              $val493 = new Ty3();
              $xfer += $val493->read($input);
              if ($key492 !== null && $val493 !== null) {
                $_val486[$key492] = $val493;
              }
            }
            $xfer += $input->readMapEnd();
            $this->ivar23 = $_val486;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 44:
          if ($ftype == TType::STRUCT) {
            $this->ivar24 = new Ty4();
            $xfer += $this->ivar24->read($input);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 45:
          if ($ftype == TType::I64) {
            $xfer += $input->readI64($this->ivar25);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 46:
          if ($ftype == TType::I32) {
            $xfer += $input->readI32($this->ivar26);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 47:
          if ($ftype == TType::LST) {
            $_size495 = 0;
            $_val494 = Vector {};
            $_etype498 = 0;
            $xfer += $input->readListBegin($_etype498, $_size495);
            for ($_i499 = 0; $_i499 < $_size495; ++$_i499)
            {
              $elem500 = null;
              $xfer += $input->readString($elem500);
              if ($elem500 !== null) {
                $_val494 []= $elem500;
              }
            }
            $xfer += $input->readListEnd();
            $this->ivar27 = $_val494;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 48:
          if ($ftype == TType::LST) {
            $_size502 = 0;
            $_val501 = Vector {};
            $_etype505 = 0;
            $xfer += $input->readListBegin($_etype505, $_size502);
            for ($_i506 = 0; $_i506 < $_size502; ++$_i506)
            {
              $elem507 = null;
              $xfer += $input->readString($elem507);
              if ($elem507 !== null) {
                $_val501 []= $elem507;
              }
            }
            $xfer += $input->readListEnd();
            $this->ivar28 = $_val501;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 49:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar29);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 50:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar30);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 51:
          if ($ftype == TType::STRUCT) {
            $this->ivar31 = new Ty5();
            $xfer += $this->ivar31->read($input);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 52:
          if ($ftype == TType::LST) {
            $_size509 = 0;
            $_val508 = Vector {};
            $_etype512 = 0;
            $xfer += $input->readListBegin($_etype512, $_size509);
            for ($_i513 = 0; $_i513 < $_size509; ++$_i513)
            {
              $elem514 = null;
              $xfer += $input->readString($elem514);
              if ($elem514 !== null) {
                $_val508 []= $elem514;
              }
            }
            $xfer += $input->readListEnd();
            $this->ivar32 = $_val508;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 53:
          if ($ftype == TType::LST) {
            $_size516 = 0;
            $_val515 = Vector {};
            $_etype519 = 0;
            $xfer += $input->readListBegin($_etype519, $_size516);
            for ($_i520 = 0; $_i520 < $_size516; ++$_i520)
            {
              $elem521 = null;
              $elem521 = new Ty2();
              $xfer += $elem521->read($input);
              if ($elem521 !== null) {
                $_val515 []= $elem521;
              }
            }
            $xfer += $input->readListEnd();
            $this->ivar33 = $_val515;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 54:
          if ($ftype == TType::I64) {
            $xfer += $input->readI64($this->ivar34);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 55:
          if ($ftype == TType::BOOL) {
            $xfer += $input->readBool($this->ivar35);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 56:
          if ($ftype == TType::LST) {
            $_size523 = 0;
            $_val522 = Vector {};
            $_etype526 = 0;
            $xfer += $input->readListBegin($_etype526, $_size523);
            for ($_i527 = 0; $_i527 < $_size523; ++$_i527)
            {
              $elem528 = null;
              $elem528 = new Ty2();
              $xfer += $elem528->read($input);
              if ($elem528 !== null) {
                $_val522 []= $elem528;
              }
            }
            $xfer += $input->readListEnd();
            $this->ivar36 = $_val522;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 57:
          if ($ftype == TType::LST) {
            $_size530 = 0;
            $_val529 = Vector {};
            $_etype533 = 0;
            $xfer += $input->readListBegin($_etype533, $_size530);
            for ($_i534 = 0; $_i534 < $_size530; ++$_i534)
            {
              $elem535 = null;
              $xfer += $input->readString($elem535);
              if ($elem535 !== null) {
                $_val529 []= $elem535;
              }
            }
            $xfer += $input->readListEnd();
            $this->ivar37 = $_val529;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 58:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar38);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 59:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar39);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 60:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar40);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 61:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar41);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 62:
          if ($ftype == TType::STRUCT) {
            $this->ivar42 = new Ty6();
            $xfer += $this->ivar42->read($input);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 63:
          if ($ftype == TType::I64) {
            $xfer += $input->readI64($this->ivar43);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 64:
          if ($ftype == TType::STRUCT) {
            $this->ivar44 = new Ty7();
            $xfer += $this->ivar44->read($input);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 65:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar45);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 66:
          if ($ftype == TType::MAP) {
            $_size537 = 0;
            $_val536 = Map {};
            $_ktype538 = 0;
            $_vtype539 = 0;
            $xfer += $input->readMapBegin($_ktype538, $_vtype539, $_size537);
            for ($_i541 = 0; $_i541 < $_size537; ++$_i541)
            {
              $key542 = '';
              $val543 = '';
              $xfer += $input->readString($key542);
              $xfer += $input->readString($val543);
              if ($key542 !== null && $val543 !== null) {
                $_val536[$key542] = $val543;
              }
            }
            $xfer += $input->readMapEnd();
            $this->ivar46 = $_val536;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 67:
          if ($ftype == TType::MAP) {
            $_size545 = 0;
            $_val544 = Map {};
            $_ktype546 = 0;
            $_vtype547 = 0;
            $xfer += $input->readMapBegin($_ktype546, $_vtype547, $_size545);
            for ($_i549 = 0; $_i549 < $_size545; ++$_i549)
            {
              $key550 = '';
              $val551 = '';
              $xfer += $input->readString($key550);
              $xfer += $input->readString($val551);
              if ($key550 !== null && $val551 !== null) {
                $_val544[$key550] = $val551;
              }
            }
            $xfer += $input->readMapEnd();
            $this->ivar47 = $_val544;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 68:
          if ($ftype == TType::LST) {
            $_size553 = 0;
            $_val552 = Vector {};
            $_etype556 = 0;
            $xfer += $input->readListBegin($_etype556, $_size553);
            for ($_i557 = 0; $_i557 < $_size553; ++$_i557)
            {
              $elem558 = null;
              $elem558 = new Ty8();
              $xfer += $elem558->read($input);
              if ($elem558 !== null) {
                $_val552 []= $elem558;
              }
            }
            $xfer += $input->readListEnd();
            $this->ivar48 = $_val552;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 69:
          if ($ftype == TType::I64) {
            $xfer += $input->readI64($this->ivar49);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 71:
          if ($ftype == TType::STRUCT) {
            $this->ivar50 = new Ty3();
            $xfer += $this->ivar50->read($input);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 72:
          if ($ftype == TType::MAP) {
            $_size560 = 0;
            $_val559 = Map {};
            $_ktype561 = 0;
            $_vtype562 = 0;
            $xfer += $input->readMapBegin($_ktype561, $_vtype562, $_size560);
            for ($_i564 = 0; $_i564 < $_size560; ++$_i564)
            {
              $key565 = 0;
              $val566 = new Ty9();
              $xfer += $input->readI32($key565);
              $val566 = new Ty9();
              $xfer += $val566->read($input);
              if ($key565 !== null && $val566 !== null) {
                $_val559[$key565] = $val566;
              }
            }
            $xfer += $input->readMapEnd();
            $this->ivar51 = $_val559;
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 73:
          if ($ftype == TType::STRUCT) {
            $this->ivar52 = new Ty10();
            $xfer += $this->ivar52->read($input);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        case 74:
          if ($ftype == TType::STRING) {
            $xfer += $input->readString($this->ivar53);
          } else {
            $xfer += $input->skip($ftype);
          }
          break;
        default:
          $xfer += $input->skip($ftype);
          break;
      }
      $xfer += $input->readFieldEnd();
    }
    $xfer += $input->readStructEnd();
    return $xfer;
  }

}
