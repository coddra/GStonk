[use("std.gst")]
;;what is the sum of all numbers below 1000 that is divisible with 3 or 5

()bruteforce():: {
    0 1 .while .dup 1000 < {
        .if .dup 3 % 0 == .over 5 % 0 == || {
            .dup .rotr + .swap
        }
        ++
    } .drop writeLine(u8):: .ret
}

()sumn(n::u8, d::u8)::u8 {
    #n::u8 #d::u8 / .dup ++ * #d::u8 * 2 / .ret
}
()solve():: {
    999 3  sumn(u8,u8)::u8
    999 5  sumn(u8,u8)::u8 +
    999 15 sumn(u8,u8)::u8 -
    writeLine(u8):: .ret
}

()main():: [main] {
    bruteforce()::
    solve()::
    .ret
}
