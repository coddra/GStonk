" Vim syntax file
" Language: gstonk

if exists("b:current_syntax")
  finish
endif

set iskeyword=a-z,A-Z,.
syntax keyword gstonkKeywords .if .else .elif .try .catch .while .ret

syntax region gstonkCommentLine start=";;" end="$"

syntax region gstonkString start=/\v"/ skip=/\v\\./ end=/\v"/ contains=gstonkEscapes

syntax region gstonkChar start=/\v'/ skip=/\v\\./ end=/\v'/ contains=gstonkEscapes

syntax match gstonkEscapes display contained "\\[nrt\\\"']"

syntax region gstonkNumber start=/\s\d/ skip=/\d/ end=/\s/

highlight default link gstonkTodos Todo
highlight default link gstonkKeywords Keyword
highlight default link gstonkCommentLine Comment
highlight default link gstonkString String
highlight default link gstonkNumber Number
highlight default link gstonkChar Character
highlight default link gstonkEscapes SpecialChar

let b:current_syntax = "gstonk"
