load ./libbuiltin[info sharedlibextension]


puts [test-num 42]
puts [test-num 3.142]
puts [test-num f123]
puts [test-num [expr true]]
puts [test-num [expr 42 * 1000000 * 1000000 * 1000000]]
puts [test-num [::binary decode hex 01234567]]
puts [test-num [list 1 2 3 4 5 ]]
puts [test-num [dict create pi 3.142 answer 4.2]]

test-num