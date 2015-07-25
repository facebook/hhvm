include <etch-masm.inc>

; Not that, when porting, the loop instructions have been changed
; to `dec ARG4_4; jnz myDest`

hash_string_i_crcSeg SEGMENT ALIGN(16) READ EXECUTE 'CODE'
hash_string_i_crc PROC
  or eax, -1
  neg ARG2_4
  je i_end
  mov ARG4_4, ARG2_4
  mov ARG5, 0dfdfdfdfdfdfdfdfh
  jmp iheader

iloop:
  add ARG1, 8
  crc32 rax, ARG3

iheader:
  mov ARG3, ARG5
  and ARG3, [ARG1]
  add ARG4_4, 8
  jnc iloop

  shl ARG4_4, 3
  xchg ARG4_1, CL
  shl ARG3, CL
  xchg CL, ARG4_1
  crc32 rax, ARG3

i_end:
  shr eax, 1
  ret
hash_string_i_crc ENDP
hash_string_i_crcSeg ENDS

hash_string_i_unaligned_crcSeg SEGMENT ALIGN(16) READ EXECUTE 'CODE'
hash_string_i_unaligned_crc PROC
  or eax, -1
  sub ARG2_4, 8
  mov ARG5, 0dfdfdfdfdfdfdfdfh
  js iutail

iuloop:
  mov ARG3, [ARG1]
  and ARG3, ARG5
  add ARG1, 8
  crc32 rax, ARG3
  sub ARG2_4, 8
  jns iuloop

iutail:
  add ARG2_4, 8
  je iuend
  mov ARG4_4, ARG2_4
  xor ARG3_4, ARG3_4

iutailloop:
  mov ARG3_1, [ARG1]
  inc ARG1
  ror ARG3, 8
  dec ARG4_4
  jnz iutailloop
  and ARG3, ARG5
  crc32 rax, ARG3

iuend:
  shr eax, 1
  ret
hash_string_i_unaligned_crc ENDP
hash_string_i_unaligned_crcSeg ENDS

hash_string_cs_crcSeg SEGMENT ALIGN(16) READ EXECUTE 'CODE'
hash_string_cs_crc PROC
  or eax, -1
  neg ARG2_4
  je csend
  mov ARG4_4, ARG2_4
  jmp csheader

csloop:
  add ARG1, 8
  crc32 rax, ARG3

csheader:
  mov ARG3, [ARG1]
  add ARG4_4, 8
  jnc csloop

  shl ARG4_4, 3
  xchg ARG4_1, CL
  shl ARG3, CL
  xchg CL, ARG4_1
  crc32 rax, ARG3

csend:
  shr eax, 1
  ret
hash_string_cs_crc ENDP
hash_string_cs_crcSeg ENDS

hash_string_cs_unaligned_crcSeg SEGMENT ALIGN(16) READ EXECUTE 'CODE'
hash_string_cs_unaligned_crc PROC
  or eax, -1
  sub ARG2_4, 8
  js csutail

csuloop:
  mov ARG3, [ARG1]
  add ARG1, 8
  crc32 rax, ARG3
  sub ARG2_4, 8
  jns csuloop

csutail:
  add ARG2_4, 8
  je csuend
  mov ARG4_4, ARG2_4
  xor ARG3_4, ARG3_4

csutailloop:
  mov ARG3_1, [ARG1]
  inc ARG1
  ror ARG3, 8
  dec ARG4_4
  jnz csutailloop
  crc32 rax, ARG3

csuend:
  shr eax, 1
  ret
hash_string_cs_unaligned_crc ENDP
hash_string_cs_unaligned_crcSeg ENDS

; The following is the asm version of HPHP::StringData::hashHelper
g_hashHelper_crcSeg SEGMENT ALIGN(16) READ EXECUTE 'CODE'
g_hashHelper_crc PROC
  mov ARG4_4, [ARG1 + 10h]
  or eax, -1
  test ARG4_4, ARG4_4
  jz hend
  mov ARG3, [ARG1]

hloop:
  mov ARG2, 0dfdfdfdfdfdfdfdfh
  and ARG2, [ARG3]
  sub ARG4_4, 8
  jle htail
  crc32 rax, ARG2
  add ARG3, 8
  jmp hloop

htail:
  shl ARG4_4, 3
  neg ARG4_4
  xchg ARG4_1, CL
  shl ARG2, CL
  xchg CL, ARG4_1
  crc32 rax, ARG2

hend:
  shr eax, 1
  or [ARG1 + 14h], eax
  ret
g_hashHelper_crc ENDP
g_hashHelper_crcSeg ENDS
END
