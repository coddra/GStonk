;; temporary pseudo stdlib for the GStonk language
::u8 (8);
::u4 (4);
::u2 (2);
::u1 (1);

::i8 [signed] (8);
::i4 [signed] (4);
::i2 [signed] (2);
::i1 [signed] (1);

::bool (1);

::d8 (8);
::d4 (4);

::string (length::u8);

::exception (message::string);

::ptr (8);

()isOverflow(flags::u8)::bool { ;;probably does not work properly
    #0 11 >> 1 & <<|
}

()new(message::string)::exception {
    8 .malloc ** #0 >8 <<|
}

()toString(x::u8)::string (res::string, ptr::ptr) {
    ?? #0 0 == {
        "0" <<|
    }

    28 .malloc ** ** >`0 1 >8 8 + >`1

    9 @@ ** #0 < {
        10 * 9 +
        `0 ** @8 ++ >8
        `1 ++ >`1
    } \\

    #0 @@ ** 0 > {
        10 /% 48 + `1 ** -- >`1 <> >1
    } \\

    `0 <<|
}
()toString(x::i8)::string (res::string, ptr::ptr) {
    ?? #0 0 == {
        "0" <<|
    }

    29 .malloc ** ** >`0
    ?? #0 0 <- {
        2 >8 8 + ** 45 >1 ++ >`1
        #0 ~ >#0
    } |> {
        1 >8 8 + >`1
    }

    9 @@ ** #0 < {
        10 * 9 +
        `0 ** @8 ++ >8
        `1 ++ >`1
    } \\

    #0 @@ ** 0 > {
       10 /% 48 + `1 ** -- >`1 <> >1
    } \\

    `0 <<|
}
()toString(x::d8, d::u1)::string (res::string, ptr::ptr, neg::u1) {
    ?? #1 0 == {
        #0 .-> toString(i8)::string <<|
    }
    ?? #0 0.0 ==. {
        #1 10 + .malloc ** ** >`0 #1 2 + >8 8 + ** 48 >1 ++ ** 46 >1 ++ >`1
        2 @@ ** `0 @8 != {
            `1 ** 48 >1 ++ >`1
            ++
        } \\
        `0 <<|
    }

    64 .malloc ** ** >`0
    ?? #0 0.0 <. {
        3 #1 + >8 8 + ** 45 >1 2 #1 + + >`1
        #0 ~. >#0
        1 >`2
    } |> {
        2 #1 + >8 9 #1 + + >`1
        0 >`2
    }

    9.0 @@ ** #0 <. {
        10.0 *. 9.0 +.
        `0 ** @8 ++ >8
        `1 ++ >`1
    } \\

    #0 10.0 #1 pow(d8,u1)::d8 *.
    .-> >#0
    0 @@ ** `2 `0 @8 -- `0 @8 ?\ < {
        ?? ** #1 == {
            `1 ** 46 >1 -- >`1
            ++
        }
        #0 10 /% <> >#0 48 + `1 ** -- >`1 <> >1
        ++
    } \\
    `0 <<|
}

()write(s::string):: {
    #0 $stdout .puts <<|
}
()writeLine(s::string):: {
    #0 $stdout .puts "\n" $stdout .puts <<|
}
()writeLine():: {
    "\n" $stdout .puts <<|
}

()write(x::u8):: {
    #0 toString(u8)::string write(string):: <<|
}
()writeLine(x::u8):: {
    #0 toString(u8)::string write(string):: writeLine():: <<|
}
()write(x::i8):: {
    #x::i8 toString(i8)::string write(string):: <<|
}
()writeLine(x::i8):: {
    #x::i8 toString(i8)::string write(string):: "\n" write(string):: <<|
}
()write(x::d8, d::u1):: {
    #0 #1 toString(d8,u1)::string write(string):: <<|
}
()writeLine(x::d8, d::u1):: {
    #0 #1 toString(d8,u1)::string write(string):: "\n" write(string):: <<|
}

()pow(x::u8, p::u1)::u8 {
    1 #1 @@ ** 0 > {
        <> #0 *. <>
        --
    } \\
    <<|
}
()pow(x::d8, p::u1)::d8 {
    1.0 #1 @@ ** 0 > {
        <> #0 *. <>
        --
    } \\

    <<|
}
