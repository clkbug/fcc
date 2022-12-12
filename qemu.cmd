define dumpReg
    set $numExecInsn = 5000
    set $i = 0
    while($i<$numExecInsn)
        set $i = $i + 1
        info registers
        x/i $pc
        stepi
	end
end

target remote localhost:12345
dumpReg
quit
y
