[use("std.gst")]
;;find the sum of all the even-valued terms in the fibonacci sequence which do not exceed four million

()bruteforce()::u8 (res::u8) {
    0 >`0
    1 1 @@ ** 4000000 <= {
        ** <>> +
        ?? ** 2 % 0 == {
            ** `0 + >`0
        }
    } \\ \\
    `0 <<|
}

()solve()::u8 (res::u8) {
    0 >`0
    0 2 @@ ** 4000000 <= {
        ** `0 + >`0 ** <<> <> 4 * +
    } \\ \\
    `0 <<|
}

()main():: [main] {
    bruteforce()::u8 writeLine(u8)::
    solve()::u8 writeLine(u8)::
    <<|
}
