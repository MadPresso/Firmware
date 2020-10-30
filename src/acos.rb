#!/bin/ruby

length = 256

length.times do |i|
  v = Math::acos(i.to_f / length.to_f) / (Math::PI / 2)
  v *= length
  v -= 1

  v = 0 if (i == length - 1)

  print "0x%02x, " % v.to_i
  print "\n" if i % 8 == 7
end