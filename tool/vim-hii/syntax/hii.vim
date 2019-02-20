" hii syntax file
" Language:     hii
" Maintainer:   s_hiiragi
" Last Change:  2017 Jul 11

syntax keyword hiTodo           contained TODO FIXME XXX

syntax region hiLineComment     start=+#+ end=+$+ contains=hiTodo
syntax region hiComment         start=+#{+ end=+#}+
syntax match  hiConstant        "\w\+\(\s*=\)\@="
syntax region hiString          start=+"+ skip=+\\"+ end=+"+
syntax match  hiNumber          "-?\d\+"

syntax keyword hiConditional    if elif else sw case
syntax keyword hiRepeat         loop
syntax keyword hiType           void bool num int str ary

syntax match hiVariable         "\$\w\+"

syntax keyword hiKeyword1       fun end ret cont break
syntax keyword hiKeyword2       _put_scopes input nop p print

highlight link hiLineComment Comment
highlight link hiComment Comment
highlight link hiConstant Constant
highlight link hiString String
highlight link hiNumber Number
"highlight link hiBoolean Boolean
"highlight link hiFloat Float
"highlight link hiIdentifier Identifier
highlight link hiVariable Identifier
"highlight link hiStatement Statement
highlight link hiConditional Conditional
highlight link hiRepeat Repeat
"highlight link hiOperator Operator
highlight link hiKeyword1 Keyword
highlight link hiKeyword2 Keyword
"highlight link hiException Exception
highlight link hiType Type
"highlight link hiStructure Structure
highlight link hiTodo Todo

let b:current_syntax = "hi"

