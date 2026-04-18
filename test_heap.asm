movi r1, #100
alloc r1, r2
printr r2

movi r3, #42
movrm r2, r3
movmr r4, r2
printr r4

freememory r2

movi r1, #50
alloc r1, r5
printr r5

movi r8, #999
printr r8
exit