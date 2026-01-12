public get_cs
public get_ds
public get_es
public get_ss
public get_fs
public get_gs
public get_ldtr
public get_tr

.code

get_cs proc
    mov rax, cs
    ret
get_cs endp

get_ds proc
    mov rax, ds
    ret
get_ds endp

get_es proc
    mov rax, es
    ret
get_es endp

get_ss proc
    mov rax, ss
    ret
get_ss endp

get_fs proc
    mov rax, fs
    ret
get_fs endp

get_gs proc
    mov rax, gs
    ret
get_gs endp

get_ldtr proc
    sldt rax
    ret
get_ldtr endp

get_tr proc
    str rax
    ret
get_tr endp

end
