/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
/*
 * Include this wherever the STATS macro is used.
 */
#define O(name, u0, u1, u2, u3) \
  STAT(Instr_InterpOne ## name) \
  STAT(Instr_InterpBB ## name) \
  STAT(Instr_TranslIRPre ## name) \
  STAT(Instr_TranslIRPost ## name) \
  STAT(Instr_Transl ## name)

#define STATS_PER_OPCODE        5
