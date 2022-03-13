# GStonk
[GStonk](https://github.com/Andrew-Reeds/GStonk) is a [concatenative](https://en.wikipedia.org/wiki/Concatenative_programming_language) [stack-oriented](https://en.wikipedia.org/wiki/Stack-oriented_programming) [intermediate](https://en.wikipedia.org/wiki/Intermediate_representation#Intermediate_language) [programming language](https://en.wikipedia.org/wiki/Programming_language).
## Disclaimer
**IMPORTANT! Gstonk is a work in progress. Anything can change at any moment.**
Also, not everything is properly tested.
And the compiler outputs assembly, which can be bothering...
## Installation
Prerequisites:
- [gcc](https://gcc.gnu.org/)
- [make](https://www.gnu.org/software/make/)
``` console
$ git clone https://github.com/Andrew-Reeds/GStonk
$ make
```
## Usage
``` console
$ gstonk <source1> [source2] [source...] [-o<output>]
```
Syntax explanations will be added later, in the meantime, be content with the examples.
## Examples
``` gstonk
()main():: [main] {
    "Hello World!\n" 1 .puts .ret
}
```

``` gstonk
::u8 (8);
::string (length::u8);
::ptr (8);

()toString(x::u8)::string (res::string, ptr::ptr) {
    ?? #0 0 == {
        "0" <<|
    }

    28 .malloc ** ** ->`0 1 ->@8 8 + ->`1

    9 @@ ** #0 < {
        10 * 9 +
        `0 ** @->8 ++ ->@8
        `1 ++ ->`1
    } \\

    #0 @@ ** 0 > {
        10 /% 48 + `1 ** -- ->`1 <> ->@1
    } \\

    `0 <<|
}

()writeLine(s::string) {
    #0 1 .puts <<|
}

()main():: [main] {
    8 11 * toString(u8)::string writeLine(string):: <<|
}
```
## Quotes
>Don't judge!
- Andrew Reeds 05/03/2022
>This didn't work...
- Andrew Reeds 05/13/2022
