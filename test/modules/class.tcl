load ./libclass[info sharedlibextension]

proc assert condition {
    puts "Testing ${condition}"
    set s "{$condition}"
    if {![uplevel 1 expr $s]} {
        return -code error "assertion failed: $condition"
    }
}

puts [test-class .get static_i]
assert {[test-class s_get]  == [test-class .get static_i] }

test-class .set static_i 42

assert {[test-class s_get] == 42 }
assert {[test-class .get static_i ] == 42 }
test-class s_set 24

assert {[test-class s_get] == 24 }
assert {[test-class .get static_i ] == 24 }

set val [test-class new 42]
puts "Created *$val"
puts [*$val .get j]
puts [*$val .set j 12]
assert {[*$val .get j] == 12}

puts [*$val test 123]
puts "Use *$val"

assert {[*$val test] == 12}
*$val test 312
#assert {[*$val test] == 312}

unset val

set val2 [test-class new 42]
puts $val2
unset val2

puts "Foobar"
