label: j 0x400000
add $3, $4, $8
addi $1, $2, 100
mult $2, $3
loop: mflo $1
sra $1, $2, 10
slt $1, $2, $3
lb $1, 100($2)
j label
j loop
j loop
