.intel_syntax noprefix
.global _decompress
/*---------------------registers----------------------------------------------------------------------------
 xmm0: Contains the 16 bytes from our bitmap we are currently working with
 cl: current byte, ch: next byte
 rdx: is used as counter
 r8: Amount of bytes that have been allocated for our resulting bitmap (argument 3)
 r9: Amount of words that were already extracted from xmm0. If r9 reaches 8 every word has been extracted and we have to get new data from the memory.
 r10: bitmap width (argument 4)
 r11: Is used to make the correct padding at the end of each line. Lines in a uncompressed bitmap have to be 4-Byte alligned.
 rax: Contains the current amount of bytes that have been written to the result memory. Before we write any new data into the result memory we have to make sure
 that there is enough memory allocated (r8)
--------------------arguments--------------------------------------------------------------------------------
1. argument: Pointer to Bitmap data (rdi).
2. argument: Pointer to the allocated result memory (rsi).
3. argument: Size of allocated result memory. (moved from rdx to r8)
4. argument: Bitmap width (moved from rcx to r10)
return value: Size of decompressed bitmap. -1 if the allocated result memory was to small.
 */

_decompress:
 MOVUPS xmm0, [rdi]
 MOV r8, rdx
 XOR r9, r9
 XOR r10, r10
 XOR r11, r11
 XOR rax, rax
 MOV r10, rcx
 XOR rcx, rcx
 XOR rdx, rdx
 XOR rbx, rbx

 CALL calculate_padding

 PEXTRW rcx, xmm0, 0 //extract the first two bytes from xmm0
 MOV r9, 1 //r9=1, because at this point only one word has been extracted from xmm0

/*------------------main loop--------------------
 Begin of the main loop. From here the value in cl decides what we have to do next.
 If cl != 0 we are dealing with encoded mode. Otherwise we have to test the value in ch.
 ------------------------------------------------*/
 main_loop:
 CMP cl, 0
 JZ test_mode
 CALL encoded_mode
 JMP continue

 test_mode:
 CMP ch, 0 //linebreak?
 JE call_linebreak

 CMP ch, 1 //end of bitmap?
 JE end_no_errors

 CMP ch, 2 //delta mode?
 JE call_delta_mode

 CALL absolute_mode //If we reach this point only absolute mode is possible.
 JMP continue

 call_linebreak:
 CALL linebreak
 JMP continue

 call_delta_mode:
 CALL delta_mode

 continue:
 CALL get_next_word
 CMP rax, -1
 JE end_error
 JMP main_loop

 end_no_errors:
 XOR ch, ch
 CALL linebreak

 end_error:
 ret

/*--------------calculate padding------------------
If the bitmap has a width that can't be devided by 4 we have to insert padding at each end of a line, so the lines are 4-Byte alligned.
caluculate_padding caluculates how many bytes of padding per line we need.
------------------------------------------------*/
calculate_padding:
 MOV rax, r10
 MOV rbx, 4
 DIV rbx
 MOV r11, rdx
 CMP r11, 0
 JE modulo_no_padding
 SUB rbx, r11
 MOV r11, rbx

 modulo_no_padding:
 XOR rax, rax
 XOR rbx, rbx
 XOR rdx, rdx
 ret

/*---------------linebreak-----------------
 We detect a linebreak when the sequence "0x00 0x00" occurs outside of a mode.
 Whenever a linebreak occurs we have to insert between 0 and 3 bytes of padding.
--------------------------------------------*/
linebreak:
 MOV rdx, r11
 loop_linebreak:
 CMP rax, r8
 JGE not_enough_memory_linebreak
 CMP rdx, 0
 JE end_linebreak
 MOV [rsi], ch
 INC rsi
 INC rax
 DEC rdx
 JMP loop_linebreak

 not_enough_memory_linebreak:
 MOV rax, -1

 end_linebreak:
 ret


/*--------------encoded mode------------------
 In encoded mode we write cl times the value in ch into the result memory.
--------------------------------------------*/
encoded_mode:
 CMP rax, r8
 JGE not_enough_memory_encoded
 MOV [rsi], ch
 INC rax
 INC rsi
 DEC cl
 CMP cl, 0
 JNZ encoded_mode
 ret

 not_enough_memory_encoded:
 MOV rax, -1
 ret

/*--------------absolute mode------------------
 In absolute mode we can transfer the data as is into the result memory. At the beginning ch contains the amount of bytes that are encoded in absolute mode.
 This sequence is always word-alligned.
--------------------------------------------*/
absolute_mode:
 MOV dl, ch
 CALL get_next_word

 loop_absolute:
 CMP rax, r8
 JGE not_enough_memory_absolute
 MOV [rsi], cl
 INC rsi
 INC rax
 DEC dl

 CMP dl, 0
 JE end_absolute
 CMP rax, r8
 JGE not_enough_memory_absolute
 MOV [rsi], ch
 INC rsi
 INC rax
 DEC dl
 CMP dl, 0
 JE end_absolute
 CALL get_next_word
 JMP loop_absolute

 not_enough_memory_absolute:
 MOV rax, -1

 end_absolute:
 ret

/*-------------delta mode-------------------//
 We detect a delta mode when the sequence "0x00 0x02" occurs outside of a mode. In delta mode there are only bytes encoded of the value 0x00.
 The amount of bytes encoded like this is specified by the two bytes that follow "0x00 0x02".
 The first byte specifies the vertical offset, the second byte specifies the horizontal offset. To get the absolute offset we have to make the following calculation:
 absolute offset = vertical offset * bitmap width + horizontal offset
 Then we just write (absolute offset)-times 0x00 into the result memory.
//------------------------------------------*/
delta_mode:
 CALL get_next_word
 XOR rdx, rdx
 MOV dl, ch //The vertical offset is stored in ch; the horizontal offset is sotred in cl.
 IMUL rdx, r10 //Multiply the vertical offset with the image width.
 XOR ch, ch
 ADD rdx, rcx //Add the horizontal offset.

 loop_delta:
 CMP rdx, 0
 JE end_delta
 CMP rax, r8
 JGE not_enough_memory_delta
 MOV [rsi], ch
 DEC rdx
 INC rax
 INC rsi
 JMP loop_delta

 not_enough_memory_delta:
 MOV rax, -1

 end_delta:
 ret

/*------------get next word--------------------
 This sub routine extracts the next word from the xmm0 resgister and stores it in CX.
 If every word has been extracted it gets the next 8 words from the memory.
---------------------------------------------------*/
get_next_word:
 CMP r9, 8
 JE xmm_register_empty
 PSRLDQ xmm0, 2
 JMP load_new_value

 xmm_register_empty:
 ADD rdi, 16
 MOVUPS xmm0,[rdi]
 XOR r9, r9

 load_new_value:
 PEXTRW ecx, xmm0, 0
 INC r9
 ret
