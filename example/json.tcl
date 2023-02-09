load ./libjson[info sharedlibextension]

set js [json::parse "{ \"foo\" : 42, \"bar\": null , \"array\" : \[1,2,3,4\], \"foobar\" : \[\] }" ]

puts "Parsed json: '$js'"
puts json::null
puts [json::serialize $js]
