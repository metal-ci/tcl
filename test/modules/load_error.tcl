package require json

catch {
    load ./libload_error[info sharedlibextension]
} err

proc assert condition {
   set s "{$condition}"
   if {![uplevel 1 expr $s]} {
       return -code error "assertion failed: $condition"
   }
}

puts "Error '$err'"
assert { $err == "my-load-error" }