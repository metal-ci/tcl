load ./libproc[info sharedlibextension]

proc assert condition {
    puts "Testing ${condition}"
    set s "{$condition}"
    if {![uplevel 1 expr $s]} {
        return -code error "assertion failed: $condition"
    }
}

proc incorrect_add {x y} {
    return [ expr "$x" + "$y" + 1]
}

set res [call-callback incorrect_add 3 4 ]

assert { $res == 8 }