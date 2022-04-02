[use("std.gst")]
;;find the sum of all the even-valued terms in the fibonacci sequence which do not exceed four million

()bruteforce()::u8 (res::u8) {
    0 >`res::u8
    1 1 .while .dup 4000000 <= {
        .dup .rotr +
        ?? .dup 2 % 0 == {
            .dup `res::u8 + >`res::u8
        }
    } .drop .drop
    `res::u8 .ret
}

()solve()::u8 (res::u8) {
    0 >`res::u8
    0 2 .while .dup 4000000 <= {
        .dup `res::u8 + >`res::u8 .dup .rotl .swap 4 * +
    } .drop .drop
    `res::u8 .ret
}

()main():: [main] {
    bruteforce()::u8 writeLine(u8)::
    solve()::u8 writeLine(u8)::
    .ret
}
