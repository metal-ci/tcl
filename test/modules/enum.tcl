load ./libenum[info sharedlibextension]


puts [enum test foo]
puts [enum test test]
puts [enum test bar]
puts [enum test invalid]
enum test


set e1 [enum make 1]
enum check $e1

set e2 [enum make foo]
enum check $e2

set e1
set e2