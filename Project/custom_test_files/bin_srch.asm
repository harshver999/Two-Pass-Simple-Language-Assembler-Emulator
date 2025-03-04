;NAME		: HARSH RAJ
;Roll NO.	: 2301AI47
;Decaration of Authorship 
;Binary Search Program to find the index value of the target 
loop: ldc r
ldnl 0
ldc l
ldnl 0
sub
brlz done
ldc l
ldnl 0
ldc r
ldnl 0
add
ldc 1
shr
ldc mid
stnl 0
ldc mid
ldnl 0
ldc array
add
ldnl 0
ldc target
ldnl 0
sub
brz found
brlz updatel
ldc mid
ldnl 0
adc -1
ldc r
stnl 0
br loop
updatel: ldc mid
ldnl 0
adc 1
ldc l
stnl 0
br loop
found: ldc mid
ldnl 0
ldc ans
stnl 0
HALT
target: data 7
ans: data 100
l: data 0
r: data 9
mid: data 100
done: HALT
array: data 6
data 4 
data 7 
data 10
data 8 
data 5 
data 9 
data 2
data 1 
data 3

