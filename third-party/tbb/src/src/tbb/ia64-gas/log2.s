// Copyright (c) 2005-2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//
//
//

	.section .text
	.align 16
	// unsigned long __TBB_machine_lg( unsigned long x );
	// r32 = x
	.proc  __TBB_machine_lg#
	.global __TBB_machine_lg#
__TBB_machine_lg:
        shr r16=r32,1	// .x
;;
        shr r17=r32,2	// ..x
	or r32=r32,r16	// xx
;;
	shr r16=r32,3	// ...xx
	or r32=r32,r17  // xxx
;;
	shr r17=r32,5	// .....xxx
	or r32=r32,r16  // xxxxx
;;
	shr r16=r32,8	// ........xxxxx
	or r32=r32,r17	// xxxxxxxx
;;
	shr r17=r32,13
	or r32=r32,r16	// 13x
;;
	shr r16=r32,21
	or r32=r32,r17	// 21x
;;
	shr r17=r32,34  
	or r32=r32,r16	// 34x
;;
	shr r16=r32,55
	or r32=r32,r17  // 55x
;;
	or r32=r32,r16  // 64x
;;
	popcnt r8=r32
;;
	add r8=-1,r8
   	br.ret.sptk.many b0	
	.endp __TBB_machine_lg#
