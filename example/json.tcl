load ./libjson[info sharedlibextension]

# Double {{ means the {json} will be a raw string
set js [json::parse {{ "foo" : 42, "bar": null , "array" : [1,2,3,4], "foobar" : [] }} ]

puts "Parsed json as tcl:       '$js'"
puts "Serialize parsed json:    [json::serialize $js]"

puts "Raw null:                 $json::null"
puts "Serializing null:         [json::serialize $json::null]"
puts "Literal null is a string: [json::serialize null]"
puts "Literal int is a string:  [json::serialize 42]"
puts "Expr int is a number:     [json::serialize [expr 42]]"

puts "Get foo element :         [dict get $js foo]"
puts "Get array element:        [dict get $js array]"
puts "Serialize array element:  [json::serialize [dict get $js array]]"
