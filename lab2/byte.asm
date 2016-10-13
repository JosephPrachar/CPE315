# byte.asm
# This MIPS assembly program will calculate the sum of a table of words
# then a table of bytes

.data

table:     .byte    1, 2, 3, 10, 20, 30, 100
table_len: .byte    7

.text
# Sum the table of bytes
#   $t0 - Length of table
#   $t1 - Ptr to table1
#   $t2 - Counter
#   $t3 - 
#   $t4 - Next value to add
#   $t5 - Value of sum
main:  lb $t0, table_len        # Load length byte into t0
       la $t1, table            # Load address of set into t1
Loop1: lb $t4, 0($t1)            # Load value at address t1 into t4
       add $t5, $t4, $t5         # Add value of t4 to running sum (t5)
       add $t1, 0x1              # Increment pointer by sizeof(byte)=1
       add $t2, 0x1              # Increment counter
       blt $t2, $t0, Loop1       # Goto Loop1 if counter < t0 (length)
       li $v0, 0x1               # Set syscall to output
       move $a0, $t5             # Set number to output with syscall
       syscall


li $v0, 0xA      # Set syscall to quit
syscall
