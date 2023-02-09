#puts $test_chan

chan puts testchan "some data"
chan puts testchan " and more data"
puts [ chan gets testchan ]
puts [ chan gets testchan ]

chan puts testchan "xyz123"
chan flush testchan