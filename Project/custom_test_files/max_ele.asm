;NAME		: HARSH RAJ
;Roll NO.	: 2301AI47
;Decaration of Authorship 
;Program to find the maximum value in an array
loop: ldc count
ldnl 0
brlz done
ldc array
add
ldnl 0
ldc maxi
ldnl 0
sub
brlz continue
ldc maxi
ldnl 0
add
ldc maxi
stnl 0
ldc count
ldnl 0
adc -1
ldc count
stnl 0
br loop
continue: ldc count
ldnl 0
adc -1
ldc count
stnl 0
br loop
done: HALT
maxi: data 0
count: data 9
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

