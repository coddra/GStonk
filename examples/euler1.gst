[use("std.gst")]
;;what is the sum of all numbers below 1000 that is divisible with 3 or 5

()bruteforce():: {
    0 1 @@ ** 1000 < {
        ?? ** 3 % 0 == ^> 5 % 0 == || {
            ** <<> + <>
        }
        ++
    } \\ writeLine(u8):: <<|
}

()sumn(n::u8, d::u8)::u8 {
    #0 #1 / ** ++ * #1 * 2 / <<|
}
()solve():: {
    999 3  sumn(u8,u8)::u8
    999 5  sumn(u8,u8)::u8 +
    999 15 sumn(u8,u8)::u8 -
    writeLine(u8):: <<|
}

()main():: [main] {
    bruteforce()::
    solve()::
    <<|
}
